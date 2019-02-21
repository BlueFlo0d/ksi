#include "dagedit_commit.h"
#include "dagedit_kernels.h"
#define INTERVAL 1
void *ksiMVCCCommitter(void *args){
        KsiEngine *e = args;
        while(1){
                if(ksiSemTryWait(&e->committingSem,INTERVAL,0)){
                        ksiWorkQueueTryFree(&e->tasks, e->nprocs+1);
                        continue;
                }
                ksiEnginePlayingLock(e);
                if(e->playing == ksiEnginePlaying){
                        if(atomic_load_explicit(&e->epoch,memory_order_consume)!=atomic_load_explicit(&e->audioEpoch,memory_order_consume))
                                ksiSemWait(&e->migratedSem);
                }
                else{
                        atomic_store_explicit(&e->audioEpoch,(atomic_load_explicit(&e->epoch,memory_order_acquire)+1)%2,memory_order_relaxed);
                }
                KsiDagEditCmd cmd = ksiSPSCCmdListDequeue(&e->syncCmds);
                while(cmd.cmd){
                        switch(cmd.cmd){
                        case ksiDagEditAddNode:{
                                void *extArgs = malloc(cmd.data.add.len);
                                memcpy(extArgs, cmd.data.add.node->extArgs, cmd.data.add.len);
                                int32_t id;
                                KsiNode *n;
                                impl_ksiEngineAddNode(e, cmd.data.add.typeFlags, extArgs, 1, &id, &n);
                                n->args = cmd.data.add.node->args;
                                break;
                        }
                        case ksiDagEditMakeWire:
                                impl_ksiEngineMakeWire(e, cmd.data.topo.srcId, cmd.data.topo.srcPort, cmd.data.topo.desId, cmd.data.topo.desPort, cmd.data.topo.gain, 1);
                                break;
                        case ksiDagEditMakeBias:
                                impl_ksiEngineMakeBias(e, cmd.data.topo.desId, cmd.data.topo.desPort, cmd.data.topo.gain, 1);
                                break;
                        case ksiDagEditDetachNodes:
                                impl_ksiEngineDetachNodes(e, cmd.data.topo.srcId, cmd.data.topo.desId, 1);
                                break;
                        case ksiDagEditRemoveNode:{
                                KsiNode *n;
                                impl_ksiEngineRemoveNode(e, cmd.data.topo.srcId, &n, 1);
                                free(ksiEngineDestroyNode(e, n, 1));
                                break;
                        }
                        case ksiDagEditRemoveWire:
                                impl_ksiEngineRemoveWire(e, cmd.data.topo.srcId, cmd.data.topo.srcPort, cmd.data.topo.desId, cmd.data.topo.desPort, 1);
                                break;
                        case ksiDagEditSendEditingCmd:
                                impl_ksiEngineSendEditingCommand(e, cmd.data.cmd.id, cmd.data.cmd.cmd, NULL, 1);
                                free(cmd.data.cmd.cmd);
                                break;
                        default:
                                fputs("Invalid command in commiter command queue!\n", stderr);
                                abort();
                        }
                        cmd = ksiSPSCCmdListDequeue(&e->syncCmds);
                }
                ksiEnginePlayingUnlock(e);
                if(e->playing == ksiEngineFinalizing)
                        break;
        }
        return NULL;
}
