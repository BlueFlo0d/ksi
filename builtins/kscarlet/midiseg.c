//output 0: freq
//output 1: gating
//No inputs. Resources are passed by editing command.
#include "midiseg.h"
#include <math.h>
#include "rbtree.h"
#include "resource.h"
#include <stdio.h>
typedef struct{
        //Data
        KsiRBTree *tree;
        KsiRBNode *current;

        //Region position info
        int32_t offset;
        int32_t segLength;
        int32_t cycleEnd;

        //Local variables
        int32_t nextEvent;//Global timestamp
        float freq;
        int32_t gating;
} plugin_env;
KsiError kscarletMidiSegEditCmd(KsiNode *n,const char *args,const char **pcli_err_str,int flag){
        plugin_env *env = (plugin_env *)n->args;
#define $freq env->freq
#define $gating env->gating
        switch(args[0]){
        case 'm':{
                args ++;
                int mid;
                int readcount = sscanf(args,"%d",&mid);
                if(readcount!=1)
                        goto syn_err;
                CHECK_VEC(mid, n->e->timeseqResources, ksiErrorTimeSeqIdNotFound);
                env->tree = n->e->timeseqResources.data[mid];
                break;
        }
        case 'p':
                break;
        case 'P':
                break;
        default:
                goto syn_err;
        }
        return ksiErrorNone;
syn_err:
        *pcli_err_str = "Invalid argument.\n"
                "Usage:m[Notes Time Sequence ID]\n"
                "      p[Playlist Time Sequence ID]\n"
                "      P[Poly count]";
        return ksiErrorSyntax;
}
void kscarletMidiSegInit(KsiNode *n){
        n->args=malloc(sizeof(plugin_env));
        plugin_env *env = (plugin_env *)n->args;
        env->tree=NULL;
        env->current=NULL;
}
void kscarletMidiSegReset(KsiNode *n){
        plugin_env *env = (plugin_env *)n->args;
        $gating = 1;
        env->current = NULL;
}
void kscarletMidiSegDestroy(KsiNode *n){
        free(n->args);
}
void kscarletMidiSeg(KsiNode *n){
        plugin_env *env = (plugin_env *)n->args;
        if(!env->current){
                if(!env->tree)
                        goto bypass;
                env->current = ksiRBTreeNextForKey(env->tree, n->e->timeStamp);
        }
        int32_t bufsize = n->e->framesPerBuffer;
        if(!env->current)
                goto bypass;
        int32_t localNextEvent = env->current->key - n->e->timeStamp;
        if(localNextEvent>bufsize)
                goto bypass;
        for(int32_t i=0;i<bufsize;i++){
                while(localNextEvent == i){
                        $gating = (env->current->data.note.velocity == 0);
                        $freq = 440.0f*powf(2, ((float)env->current->data.note.tone-69)/12);
                        env->current = ksiRBTreeNext(env->tree,env->current);
                        if(env->current){
                                localNextEvent = env->current->key - n->e->timeStamp;
                        }
                        else
                                localNextEvent = -1;
                }
                n->outputBuffer[1].d[i].i = $gating;
                n->outputBuffer[0].d[i].f = $freq;
        }
        ksiNodePortIOSetDirty(n->outputTypes[0]);
        ksiNodePortIOSetDirty(n->outputTypes[1]);
        return;
bypass:
        n->outputBuffer[1].d[bufsize-1].i = $gating;
        n->outputBuffer[0].d[bufsize-1].f = $freq;
        ksiNodePortIOClear(n->outputTypes[0]);
        ksiNodePortIOClear(n->outputTypes[1]);
        return;
}
