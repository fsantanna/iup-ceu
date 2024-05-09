
    #include <stdio.h>
    #include <stdlib.h>
    #include <stddef.h>
    #include <stdint.h>
    #include <string.h>
    #include <assert.h>
    #include <stdarg.h>
    #include <time.h>
    #include <math.h>
    
    #define CEU_DEBUG
    #define CEU 99
    #define CEU_MULTI -1
    
    #undef MAX
    #undef MIN
    #define MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
    #define MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

    #define COMMA ,
    #if CEU >= 2
    #define CEU2(x) x
    #else
    #define CEU2(x)
    #endif
    #if CEU >= 3
    #define CEU3(x) x
    #else
    #define CEU3(x)
    #endif
    #if CEU >= 4
    #define CEU4(x) x
    #else
    #define CEU4(x)
    #endif
    #if CEU >= 5
    #define CEU5(x) x
    #else
    #define CEU5(x)
    #endif
    
    #if CEU >= 3
    typedef enum CEU_ACTION {
        CEU_ACTION_INVALID = -1,    // default to force set
        CEU_ACTION_CALL,
        CEU_ACTION_RESUME,
        CEU_ACTION_ABORT,           // awake exe to finalize defers and release memory
    #if CEU >= 4
        //CEU_ACTION_TOGGLE,          // restore time to CEU_TIME_MIN after toggle
        CEU_ACTION_ERROR,           // awake task to catch error from nested task
    #endif
    } CEU_ACTION;
    #endif

    typedef enum CEU_VALUE {
        CEU_VALUE_BLOCK = -1,
        CEU_VALUE_NIL = 0,
        CEU_VALUE_ERROR,
        CEU_VALUE_TAG,
        CEU_VALUE_BOOL,
        CEU_VALUE_CHAR,
        CEU_VALUE_NUMBER,
        CEU_VALUE_POINTER,
        CEU_VALUE_DYNAMIC,    // all below are dynamic
        CEU_VALUE_CLO_FUNC,
        #if CEU >= 3
        CEU_VALUE_CLO_CORO,
        #endif
        #if CEU >= 4
        CEU_VALUE_CLO_TASK,
        #endif
        CEU_VALUE_TUPLE,
        CEU_VALUE_VECTOR,
        CEU_VALUE_DICT,
        #if CEU >= 3
        CEU_VALUE_EXE_CORO,
        #endif
        #if CEU >= 4
        CEU_VALUE_EXE_TASK,
        #endif
        #if CEU >= 5
        CEU_VALUE_TASKS,
        CEU_VALUE_TRACK,
        #endif
        CEU_VALUE_MAX
    } __attribute__ ((__packed__)) CEU_VALUE;
    _Static_assert(sizeof(CEU_VALUE) == 1, "bug found");

    #if CEU >= 3
    typedef enum CEU_EXE_STATUS {
        CEU_EXE_STATUS_YIELDED = 1,
        #if CEU >= 4
        CEU_EXE_STATUS_TOGGLED,
        #endif
        CEU_EXE_STATUS_RESUMED,
        CEU_EXE_STATUS_TERMINATED,
    } CEU_EXE_STATUS;
    #endif
    
    #if CEU >= 4
    struct CEU_Exe_Task;
    typedef union CEU_Dyn* CEU_Block;
    #endif
    #if CEU >= 5
    struct CEU_Tasks;
    #endif
    
    typedef struct CEU_Value {
        CEU_VALUE type;
        union {
            //void nil;
            CEU4(CEU_Block Block;)
            char* Error;            // NULL=value on stack, !NULL=value is this string
            unsigned int Tag;
            int Bool;
            char Char;
            double Number;
            void* Pointer;
            union CEU_Dyn* Dyn;    // Func/Task/Tuple/Dict/Coro/Tasks: allocates memory
        };
    } CEU_Value;

    #define _CEU_Dyn_                   \
        CEU_VALUE type;                 \
        uint8_t refs;                   \
        struct CEU_Tags_List* tags;
        
    #if 0
        struct {                        \
            void* block;   /* block/tasks */ \
            union CEU_Dyn* prev;        \
            union CEU_Dyn* next;        \
        } hld;
    #endif
        
    typedef struct CEU_Any {
        _CEU_Dyn_
    } CEU_Any;

    typedef struct CEU_Tuple {
        _CEU_Dyn_
        int its;                // number of items
        CEU_Value buf[0];       // beginning of CEU_Value[n]
    } CEU_Tuple;

    typedef struct CEU_Vector {
        _CEU_Dyn_
        CEU_VALUE unit;         // type of each element
        int max;                // size of buf
        int its;                // number of items
        char* buf;              // resizable Unknown[n]
    } CEU_Vector;
    
    typedef struct CEU_Dict {
        _CEU_Dyn_
        int max;                // size of buf
        CEU_Value (*buf)[0][2]; // resizable CEU_Value[n][2]
    } CEU_Dict;

    struct CEUX;
    typedef int (*CEU_Proto) (struct CEUX* X);

    #define _CEU_Clo_                   \
        _CEU_Dyn_                       \
        CEU_Proto proto;                \
        int args;                       \
        int locs;                       \
        struct {                        \
            int its;                    \
            CEU_Value* buf;             \
        } upvs;

    typedef struct CEU_Clo {
        _CEU_Clo_
    } CEU_Clo;
    
    #if CEU >= 4
    typedef struct CEU_Clo_Task {
        _CEU_Clo_                       \
        struct CEU_Exe_Task* up_tsk;    \
    } CEU_Clo_Task;
    #endif
    
    #if CEU >= 2
    typedef struct CEU_Throw {
        _CEU_Dyn_
        CEU_Value val;
        CEU_Value stk;
    } CEU_Throw;
    #endif
    
    #if CEU >= 3
    #define _CEU_Exe_                   \
        _CEU_Dyn_                       \
        CEU_EXE_STATUS status;          \
        CEU_Value clo;                  \
        /*struct CEU_Frame frame;*/     \
        int pc;                         \
        struct CEUX* X;
        
    typedef struct CEU_Exe {
        _CEU_Exe_
    } CEU_Exe;
    #endif
    
    #if CEU >= 4
    typedef struct CEU_Links {
        struct {
            union CEU_Dyn* dyn;
            CEU_Block* blk;
        } up;
        struct {
            union CEU_Dyn* prv;
            union CEU_Dyn* nxt;
        } sd;
        struct {
            union CEU_Dyn* fst;
            union CEU_Dyn* lst;
        } dn;
    } CEU_Links;
    #endif
    
    #if CEU >= 5
        #define CEU_LNKS(dyn) ((dyn)->Any.type==CEU_VALUE_TASKS ? &(dyn)->Tasks.lnks : &(dyn)->Exe_Task.lnks)
    #else
        #define CEU_LNKS(dyn) (&((CEU_Exe_Task*) dyn)->lnks)
    #endif

    #if CEU >= 4
    typedef struct CEU_Exe_Task {
        _CEU_Exe_
        uint32_t time;      // last sleep time, only awakes if CEU_TIME>time 
        CEU_Value pub;
        CEU_Links lnks;
    } CEU_Exe_Task;
    #endif
    
    #if CEU >= 5
    typedef struct CEU_Tasks {
        _CEU_Dyn_
        int max;
        CEU_Links lnks;
    } CEU_Tasks;
    typedef struct CEU_Track {
        _CEU_Dyn_
        CEU_Exe_Task* task;
    } CEU_Track;
    #endif

    typedef union CEU_Dyn {                                                                 
        struct CEU_Any      Any;
        struct CEU_Tuple    Tuple;
        struct CEU_Vector   Vector;
        struct CEU_Dict     Dict;
        struct CEU_Clo      Clo;
    #if CEU >= 2
        struct CEU_Throw    Throw;
    #endif
    #if CEU >= 4
        struct CEU_Clo_Task Clo_Task;
    #endif
    #if CEU >= 3
        struct CEU_Exe      Exe;
    #endif
    #if CEU >= 4
        struct CEU_Exe_Task Exe_Task;
    #endif
    #if CEU >= 5
        struct CEU_Tasks    Tasks;
        struct CEU_Track    Track;
    #endif
    } CEU_Dyn;        
    
    typedef struct CEU_Tags_Names {
        int tag;
        char* name;
        struct CEU_Tags_Names* next;
    } CEU_Tags_Names;
    
    typedef struct CEU_Tags_List {
        int tag;
        struct CEU_Tags_List* next;
    } CEU_Tags_List;
    
    #define CEU_STACK_MAX 256
    typedef struct CEU_Stack {
        int n;
        CEU_Value buf[CEU_STACK_MAX];
    } CEU_Stack;
    
    typedef struct CEUX {
        CEU_Stack* S;
        int base;   // index above args
        int args;   // number of args
    #if CEU >= 3
        CEU_ACTION action;
        union {
            struct CEU_Exe* exe;
    #if CEU >= 4
            struct CEU_Exe_Task* exe_task;
    #endif
        };
    #if CEU >= 4
        uint32_t now;
        struct CEUX* up;
    #endif
    #endif
    } CEUX;
    
    /*
     *  CLO
     *  args
     *  ----    <-- base
     *  upvs
     *  locs    <-- [b1,n1[ [b2,n2[ [...[
     *  block
     *  tmps
     *  block
     *  tmps
     */
    
    #define ceux_arg(X,i) (X->base - X->args + i)
    #define XX(v)  ({ assert(v<=0); X->S->n+v; })
    #define XX1(v) ({ assert(v<=0); X1->S->n+v; })
    #define XX2(v) ({ assert(v<=0); X2->S->n+v; })
    #define SS(v)  ({ assert(v<=0); S->n+v;    })
    int ceux_n_get (CEU_Stack* S);
    void ceux_n_set (CEU_Stack* S, int base);
    CEU_Value ceux_pop (CEU_Stack* S, int dec);
    int ceux_push (CEU_Stack* S, int inc, CEU_Value v);
    CEU_Value ceux_peek (CEU_Stack* S, int i);
    void ceux_repl (CEU_Stack* S, int i, CEU_Value v);
    void ceux_ins (CEU_Stack* S, int i, CEU_Value v);
    void ceux_rem (CEU_Stack* S, int i);
    void ceux_drop (CEU_Stack* S, int n);
    #if CEU >= 3
    int ceux_resume (CEUX* X1, int inp, int out, CEU_ACTION act CEU4(COMMA uint32_t now));
    #endif
    
    CEUX* CEU_GLOBAL_X = NULL;
    #if CEU >= 4
    uint32_t CEU_TIME = 0;
    CEU_Exe_Task CEU_GLOBAL_TASK = {
        CEU_VALUE_EXE_TASK, 1, NULL,
        CEU_EXE_STATUS_YIELDED, {}, 0, NULL,
        0, {}, { {NULL,NULL}, {NULL,NULL}, {NULL,NULL} }
    };
    #endif
    int CEU_BREAK = 0;
    
    int ceu_type_f (CEUX* X);
    int ceu_as_bool (CEU_Value v);
    CEU_Value ceu_dyn_to_val (CEU_Dyn* dyn);

    int ceu_tags_f (CEUX* X);
    int ceu_type_to_size (int type);

    void ceu_gc_inc_val (CEU_Value v);

    CEU_Value ceu_create_tuple   (int n);
    CEU_Value ceu_create_vector  (void);
    CEU_Value ceu_create_dict    (void);
    CEU_Value ceu_create_clo     (CEU_VALUE type, CEU_Proto proto, int args, int locs, int upvs);
    #if CEU >= 4
    CEU_Value ceu_create_exe_task (CEU_Value clo, CEU_Dyn* up_dyn, CEU_Block* up_blk);
    CEU_Value ceu_create_track   (CEU_Exe_Task* task);
    #endif

    void ceu_tuple_set (CEU_Tuple* tup, int i, CEU_Value v);

    CEU_Value ceu_vector_get (CEU_Vector* vec, int i);
    void ceu_vector_set (CEU_Vector* vec, int i, CEU_Value v);
    CEU_Value ceu_vector_from_c_string (const char* str);
    
    int ceu_dict_key_to_index (CEU_Dict* col, CEU_Value key, int* idx);
    CEU_Value ceu_dict_get (CEU_Dict* col, CEU_Value key);
    CEU_Value ceu_dict_set (CEU_Dict* col, CEU_Value key, CEU_Value val);
    CEU_Value ceu_col_check (CEU_Value col, CEU_Value idx);

    void ceu_print1 (CEU_Value v);
    CEU_Value _ceu_equals_equals_ (CEU_Value e1, CEU_Value e2);

    char* ceu_to_dash_string_dash_tag (int tag);
    #if CEU >= 3
    int ceu_isexe_dyn (CEU_Dyn* dyn);
    int ceu_isexe_val (CEU_Value val);
    void ceu_abort_exe (CEU_Exe* exe);
    #endif
    #if CEU >= 4
    #define ceu_abort_dyn(a) ceu_abort_exe((CEU_Exe*)a)
    #define ceu_bcast_dyn(a,b,c,d) ceu_bcast_task(a,b,c,(CEU_Exe_Task*)d)
    int ceu_bcast_task (CEUX* X1, CEU_ACTION act, uint32_t now, CEU_Exe_Task* tsk2);
    int ceu_bcast_tasks (CEUX* X1, CEU_ACTION act, uint32_t now, CEU_Dyn* dyn2);
    int ceu_istask_dyn (CEU_Dyn* dyn);
    int ceu_istask_val (CEU_Value val);
    void ceu_dyn_unlink (CEU_Dyn* dyn);
    #endif
    #if CEU >= 5
    #undef ceu_abort_dyn
    #define ceu_abort_dyn(a) (a->Any.type==CEU_VALUE_TASKS ? ceu_abort_tasks((CEU_Tasks*)a) : ceu_abort_exe((CEU_Exe*)a))
    #undef ceu_bcast_dyn
    #define ceu_bcast_dyn(a,b,c,d) (d->Any.type==CEU_VALUE_TASKS ? ceu_bcast_tasks(a,b,c,d) : ceu_bcast_task(a,b,c,(CEU_Exe_Task*)d))
    void ceu_abort_tasks (CEU_Tasks* tsks);
    #endif
    
#ifdef CEU_DEBUG
    struct {
        int alloc;
        int free;
    } CEU_GC = { 0, 0 };
    
    void ceu_dump_gc (void) {
        printf(">>> GC: %d\n", CEU_GC.alloc - CEU_GC.free);
        printf("    alloc = %d\n", CEU_GC.alloc);
        printf("    free  = %d\n", CEU_GC.free);
    }
    #if 0
    void ceu_dump_frame (CEU_Frame* frame) {
        printf(">>> FRAME: %p\n", frame);
        printf("    up_block = %p\n", frame->up_block);
        printf("    clo      = %p\n", frame->clo);
    #if CEU >= 4
        printf("    exe      = %p\n", frame->exe);
    #endif
    }
    #endif
    void ceu_dump_val (CEU_Value v) {
        puts(">>>>>>>>>>>");
        ceu_print1(v);
        puts(" <<<");
        if (v.type > CEU_VALUE_DYNAMIC) {
            printf("    dyn   = %p\n", v.Dyn);
            printf("    type  = %d\n", v.type);
            printf("    refs  = %d\n", v.Dyn->Any.refs);
            //printf("    next  = %p\n", v.Dyn->Any.hld.next);
            printf("    ----\n");
            switch (v.type) {
        #if CEU >= 4
                case CEU_VALUE_EXE_TASK:
                    printf("    status = %d\n", v.Dyn->Exe_Task.status);
                    printf("    pc     = %d\n", v.Dyn->Exe_Task.pc);
                    printf("    pub    = %d\n", v.Dyn->Exe_Task.pub.type);
                    break;
        #endif
        #if CEU >= 5
                case CEU_VALUE_TASKS:
                    //printf("    up_blk = %p\n", v.Dyn->Tasks.up_blk);
                    //printf("    dn_tsk = %p\n", v.Dyn->Tasks.dn_tsk);
                    break;
                case CEU_VALUE_TRACK:
                    printf("    task   = %p\n", v.Dyn->Track.task);
                    break;
        #endif
                default:
                    puts("TODO");
            }
        }
        puts("<<<<<<<<<<<");
    }
    void ceu_dump_dyn (CEU_Dyn* dyn) {
        ceu_dump_val(ceu_dyn_to_val(dyn));
    }
    #if 0
    void ceu_dump_block (CEU_Block* blk) {
        printf(">>> BLOCK: %p\n", blk);
        printf("    istop = %d\n", blk->istop);
        //printf("    up    = %p\n", blk->up.frame);
        CEU_Dyn* cur = blk->dn.dyns.first;
        while (cur != NULL) {
            ceu_dump_dyn(cur);
            CEU_Dyn* old = cur;
            //cur = old->Any.hld.next;
        }
    }
    #endif
#endif
    
    #define CEU_ERROR_IS(S) ((S)->n>0 && ceux_peek((S),(S)->n-1).type==CEU_VALUE_ERROR)
    #define CEU_ERROR_CHK_VAL(cmd,v,pre) ({     \
        if (v.type == CEU_VALUE_ERROR) {        \
            ceu_error_e(X->S,v);                \
            CEU_ERROR_CHK_STK(cmd,pre);         \
        };                                      \
        v;                                      \
    })
    #define CEU_ERROR_THR_S(cmd,msg,pre) {      \
        ceu_error_s(X->S, msg);                 \
        CEU_ERROR_CHK_STK(cmd,pre);             \
    }

    #if CEU <= 1
    #define CEU_ERROR_CHK_STK(cmd,pre) {                                            \
        if (CEU_ERROR_IS(X->S)) {                                                   \
            CEU_Value msg = ceux_peek(X->S, XX(-2));                                \
            assert(msg.type==CEU_VALUE_POINTER && msg.Pointer!=NULL);               \
            fprintf(stderr, " |  %s\n v  error : %s\n", pre, (char*) msg.Pointer);  \
            ceux_n_set(X->S, 0);                                                    \
            exit(0);                                                                \
        }                                                                           \
    }
    #else
    #define CEU_ERROR_CHK_STK(cmd,pre)      \
        if (ceu_error_chk_stk(X->S, pre)) { \
            cmd;                            \
        }
    int ceu_error_chk_stk (CEU_Stack* S, char* pre) {
        if (!CEU_ERROR_IS(S)) {
            return 0;
        } else {
            if (pre != NULL) {      // blocks check but do not add a message
                // [...,n,pay,err]
                CEU_Value n = ceux_peek(S, SS(-3));
                assert(n.type == CEU_VALUE_NUMBER);
                ceux_repl(S, SS(-3), (CEU_Value) { CEU_VALUE_NUMBER, {.Number=n.Number+1} });
                ceux_ins(S, SS(-3), (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=pre} });
                // [...,pre,n+1,pay,err]
            }
            return 1;
        }
    }
    #endif

    int ceu_error_e (CEU_Stack* S, CEU_Value e) {
        assert(e.type==CEU_VALUE_ERROR && e.Error!=NULL);
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=e.Error} });
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_ERROR, {.Error=NULL} });
        return 3;
    }
    int ceu_error_s (CEU_Stack* S, char* s) {
        assert(s != NULL);
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=s} });
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_ERROR });
        return 3;
    }
    int ceu_error_v (CEU_Stack* S, CEU_Value v) {
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
        ceux_push(S, 1, v);
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_ERROR, {.Error=NULL} });
        return 3;
    }

    int ceu_error_f (CEUX* X) {
        assert(X->args == 1);
    #if CEU < 2
        CEU_Value arg = ceux_peek(X->S, ceux_arg(X,0));
        assert(arg.type == CEU_VALUE_TAG);
        return ceu_error_s(X->S, ceu_to_dash_string_dash_tag(arg.Tag));
    #else
        return ceu_error_v(X->S, ceux_peek(X->S, ceux_arg(X,0)));
    #endif
    }        
    
#ifdef CEU_DEBUG
    int CEU_DEBUG_TYPE[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    void ceu_debug_add (int type) {
    #ifdef CEU_DEBUG
        CEU_GC.alloc++;
    #endif
        CEU_DEBUG_TYPE[type]++;
        //printf(">>> type = %d | count = %d\n", type, CEU_DEBUG_TYPE[type]);
    }
    void ceu_debug_rem (int type) {
    #ifdef CEU_DEBUG
        CEU_GC.free++;
    #endif
        CEU_DEBUG_TYPE[type]--;
        //printf(">>> type = %d | count = %d\n", type, CEU_DEBUG_TYPE[type]);
    }
#else
    #define ceu_debug_add(x)
    #define ceu_debug_rem(x)
#endif

    void ceu_gc_free (CEU_Dyn* dyn);
    
    void ceu_gc_dec_dyn (CEU_Dyn* dyn) {
        assert(dyn->Any.refs > 0);
        dyn->Any.refs--;
        if (dyn->Any.refs == 0) {
            ceu_gc_free(dyn);
        }
    }
    void ceu_gc_dec_val (CEU_Value val) {
        if (val.type < CEU_VALUE_DYNAMIC)
            return;
        ceu_gc_dec_dyn(val.Dyn);
    }

    void ceu_gc_inc_dyn (CEU_Dyn* dyn) {
        assert(dyn->Any.refs < 255);
        dyn->Any.refs++;
    }
    void ceu_gc_inc_val (CEU_Value val) {
        if (val.type < CEU_VALUE_DYNAMIC)
            return;
        ceu_gc_inc_dyn(val.Dyn);
    }

    void ceu_gc_free (CEU_Dyn* dyn) {
        while (dyn->Any.tags != NULL) {
            CEU_Tags_List* tag = dyn->Any.tags;
            dyn->Any.tags = tag->next;
            free(tag);
        }
        switch (dyn->Any.type) {
            case CEU_VALUE_CLO_FUNC:
#if CEU >= 3
            case CEU_VALUE_CLO_CORO:
#endif
#if CEU >= 4
            case CEU_VALUE_CLO_TASK:
#endif
                for (int i=0; i<dyn->Clo.upvs.its; i++) {
                    ceu_gc_dec_val(dyn->Clo.upvs.buf[i]);
                }
                free(dyn->Clo.upvs.buf);
                break;
            case CEU_VALUE_TUPLE:       // buf w/ dyn
                for (int i=0; i<dyn->Tuple.its; i++) {
                    ceu_gc_dec_val(dyn->Tuple.buf[i]);
                }
                break;
            case CEU_VALUE_VECTOR:
                for (int i=0; i<dyn->Vector.its; i++) {
                    CEU_Value ret = ceu_vector_get(&dyn->Vector, i);
                    assert(ret.type != CEU_VALUE_ERROR);
                    ceu_gc_dec_val(ret);
                }
                free(dyn->Vector.buf);
                break;
            case CEU_VALUE_DICT:
                for (int i=0; i<dyn->Dict.max; i++) {
                    ceu_gc_dec_val((*dyn->Dict.buf)[i][0]);
                    ceu_gc_dec_val((*dyn->Dict.buf)[i][1]);
                }
                free(dyn->Dict.buf);
                break;
#if CEU >= 3
            case CEU_VALUE_EXE_CORO: {
#if CEU >= 4
            case CEU_VALUE_EXE_TASK:
#endif
                if (dyn->Exe.status != CEU_EXE_STATUS_TERMINATED) {
                    dyn->Any.refs++;            // currently 0->1: needs ->2 to prevent double gc
                    ceu_abort_exe((CEU_Exe*)dyn);
                    dyn->Any.refs--;
                }
                ceux_n_set(dyn->Exe.X->S, 0);
                ceu_gc_dec_val(dyn->Exe.clo);
#if CEU >= 4
                if (dyn->Any.type == CEU_VALUE_EXE_TASK) {
                    ceu_gc_dec_val(((CEU_Exe_Task*)dyn)->pub);
                    ceu_dyn_unlink(dyn);
                }
#endif
                free(dyn->Exe.X->S);
                free(dyn->Exe.X);
                break;
            }
#endif
#if CEU >= 5
            case CEU_VALUE_TASKS: {
                ceu_abort_tasks(&dyn->Tasks);
                ceu_dyn_unlink(dyn);
                break;
            }
            case CEU_VALUE_TRACK:
                break;
#endif
            default:
                assert(0 && "bug found");
        }
        ceu_debug_rem(dyn->Any.type);
        free(dyn);
    }        
    
                #define CEU_TAG_nil (0)
                CEU_Tags_Names ceu_tag_nil = { CEU_TAG_nil, ":nil", NULL };
                
                #define CEU_TAG_error (1)
                CEU_Tags_Names ceu_tag_error = { CEU_TAG_error, ":error", &ceu_tag_nil };
                
                #define CEU_TAG_tag (2)
                CEU_Tags_Names ceu_tag_tag = { CEU_TAG_tag, ":tag", &ceu_tag_error };
                
                #define CEU_TAG_bool (3)
                CEU_Tags_Names ceu_tag_bool = { CEU_TAG_bool, ":bool", &ceu_tag_tag };
                
                #define CEU_TAG_char (4)
                CEU_Tags_Names ceu_tag_char = { CEU_TAG_char, ":char", &ceu_tag_bool };
                
                #define CEU_TAG_number (5)
                CEU_Tags_Names ceu_tag_number = { CEU_TAG_number, ":number", &ceu_tag_char };
                
                #define CEU_TAG_pointer (6)
                CEU_Tags_Names ceu_tag_pointer = { CEU_TAG_pointer, ":pointer", &ceu_tag_number };
                
                #define CEU_TAG_dynamic (7)
                CEU_Tags_Names ceu_tag_dynamic = { CEU_TAG_dynamic, ":dynamic", &ceu_tag_pointer };
                
                #define CEU_TAG_func (8)
                CEU_Tags_Names ceu_tag_func = { CEU_TAG_func, ":func", &ceu_tag_dynamic };
                
                #define CEU_TAG_coro (9)
                CEU_Tags_Names ceu_tag_coro = { CEU_TAG_coro, ":coro", &ceu_tag_func };
                
                #define CEU_TAG_task (10)
                CEU_Tags_Names ceu_tag_task = { CEU_TAG_task, ":task", &ceu_tag_coro };
                
                #define CEU_TAG_tuple (11)
                CEU_Tags_Names ceu_tag_tuple = { CEU_TAG_tuple, ":tuple", &ceu_tag_task };
                
                #define CEU_TAG_vector (12)
                CEU_Tags_Names ceu_tag_vector = { CEU_TAG_vector, ":vector", &ceu_tag_tuple };
                
                #define CEU_TAG_dict (13)
                CEU_Tags_Names ceu_tag_dict = { CEU_TAG_dict, ":dict", &ceu_tag_vector };
                
                #define CEU_TAG_exe_coro (14)
                CEU_Tags_Names ceu_tag_exe_coro = { CEU_TAG_exe_coro, ":exe-coro", &ceu_tag_dict };
                
                #define CEU_TAG_exe_task (15)
                CEU_Tags_Names ceu_tag_exe_task = { CEU_TAG_exe_task, ":exe-task", &ceu_tag_exe_coro };
                
                #define CEU_TAG_tasks (16)
                CEU_Tags_Names ceu_tag_tasks = { CEU_TAG_tasks, ":tasks", &ceu_tag_exe_task };
                
                #define CEU_TAG_yielded (17)
                CEU_Tags_Names ceu_tag_yielded = { CEU_TAG_yielded, ":yielded", &ceu_tag_tasks };
                
                #define CEU_TAG_toggled (18)
                CEU_Tags_Names ceu_tag_toggled = { CEU_TAG_toggled, ":toggled", &ceu_tag_yielded };
                
                #define CEU_TAG_resumed (19)
                CEU_Tags_Names ceu_tag_resumed = { CEU_TAG_resumed, ":resumed", &ceu_tag_toggled };
                
                #define CEU_TAG_terminated (20)
                CEU_Tags_Names ceu_tag_terminated = { CEU_TAG_terminated, ":terminated", &ceu_tag_resumed };
                
                #define CEU_TAG_global (21)
                CEU_Tags_Names ceu_tag_global = { CEU_TAG_global, ":global", &ceu_tag_terminated };
                
                #define CEU_TAG_nested (22)
                CEU_Tags_Names ceu_tag_nested = { CEU_TAG_nested, ":nested", &ceu_tag_global };
                
                #define CEU_TAG_ceu (23)
                CEU_Tags_Names ceu_tag_ceu = { CEU_TAG_ceu, ":ceu", &ceu_tag_nested };
                
                #define CEU_TAG_pre (24)
                CEU_Tags_Names ceu_tag_pre = { CEU_TAG_pre, ":pre", &ceu_tag_ceu };
                
                #define CEU_TAG_rec (25)
                CEU_Tags_Names ceu_tag_rec = { CEU_TAG_rec, ":rec", &ceu_tag_pre };
                
                #define CEU_TAG_h (26)
                CEU_Tags_Names ceu_tag_h = { CEU_TAG_h, ":h", &ceu_tag_rec };
                
                #define CEU_TAG_min (27)
                CEU_Tags_Names ceu_tag_min = { CEU_TAG_min, ":min", &ceu_tag_h };
                
                #define CEU_TAG_s (28)
                CEU_Tags_Names ceu_tag_s = { CEU_TAG_s, ":s", &ceu_tag_min };
                
                #define CEU_TAG_ms (29)
                CEU_Tags_Names ceu_tag_ms = { CEU_TAG_ms, ":ms", &ceu_tag_s };
                
                #define CEU_TAG_Clock (30)
                CEU_Tags_Names ceu_tag_Clock = { CEU_TAG_Clock, ":Clock", &ceu_tag_ms };
                
                #define CEU_TAG_assert (31)
                CEU_Tags_Names ceu_tag_assert = { CEU_TAG_assert, ":assert", &ceu_tag_Clock };
                
                #define CEU_TAG_Iterator (32)
                CEU_Tags_Names ceu_tag_Iterator = { CEU_TAG_Iterator, ":Iterator", &ceu_tag_assert };
                
                #define CEU_TAG_i (33)
                CEU_Tags_Names ceu_tag_i = { CEU_TAG_i, ":i", &ceu_tag_Iterator };
                
                #define CEU_TAG_tp (34)
                CEU_Tags_Names ceu_tag_tp = { CEU_TAG_tp, ":tp", &ceu_tag_i };
                
                #define CEU_TAG_all (35)
                CEU_Tags_Names ceu_tag_all = { CEU_TAG_all, ":all", &ceu_tag_tp };
                
                #define CEU_TAG_idx (36)
                CEU_Tags_Names ceu_tag_idx = { CEU_TAG_idx, ":idx", &ceu_tag_all };
                
                #define CEU_TAG_val (37)
                CEU_Tags_Names ceu_tag_val = { CEU_TAG_val, ":val", &ceu_tag_idx };
                
                #define CEU_TAG_key (38)
                CEU_Tags_Names ceu_tag_key = { CEU_TAG_key, ":key", &ceu_tag_val };
                
                #define CEU_TAG_f (39)
                CEU_Tags_Names ceu_tag_f = { CEU_TAG_f, ":f", &ceu_tag_key };
                
                #define CEU_TAG_TODO (40)
                CEU_Tags_Names ceu_tag_TODO = { CEU_TAG_TODO, ":TODO", &ceu_tag_f };
                
                #define CEU_TAG_Iup (41)
                CEU_Tags_Names ceu_tag_Iup = { CEU_TAG_Iup, ":Iup", &ceu_tag_TODO };
                
                    #define CEU_TAG_Iup_Action ((1 << 8) | 41)
                    CEU_Tags_Names ceu_tag_Iup_Action = { CEU_TAG_Iup_Action, ":Iup.Action", &ceu_tag_Iup };
                    
                    #define CEU_TAG_Iup_Value ((2 << 8) | 41)
                    CEU_Tags_Names ceu_tag_Iup_Value = { CEU_TAG_Iup_Value, ":Iup.Value", &ceu_tag_Iup_Action };
                    
                    #define CEU_TAG_Iup_List ((3 << 8) | 41)
                    CEU_Tags_Names ceu_tag_Iup_List = { CEU_TAG_Iup_List, ":Iup.List", &ceu_tag_Iup_Value };
                    
                        #define CEU_TAG_Iup_List_Action ((1 << 16) | (3 << 8) | 41)
                        CEU_Tags_Names ceu_tag_Iup_List_Action = { CEU_TAG_Iup_List_Action, ":Iup.List.Action", &ceu_tag_Iup_List };
                        
                #define CEU_TAG_SIZE (42)
                CEU_Tags_Names ceu_tag_SIZE = { CEU_TAG_SIZE, ":SIZE", &ceu_tag_Iup_List_Action };
                
                #define CEU_TAG_READONLY (43)
                CEU_Tags_Names ceu_tag_READONLY = { CEU_TAG_READONLY, ":READONLY", &ceu_tag_SIZE };
                
                #define CEU_TAG_VALUE (44)
                CEU_Tags_Names ceu_tag_VALUE = { CEU_TAG_VALUE, ":VALUE", &ceu_tag_READONLY };
                
                #define CEU_TAG_MARGIN (45)
                CEU_Tags_Names ceu_tag_MARGIN = { CEU_TAG_MARGIN, ":MARGIN", &ceu_tag_VALUE };
                
                #define CEU_TAG_GAP (46)
                CEU_Tags_Names ceu_tag_GAP = { CEU_TAG_GAP, ":GAP", &ceu_tag_MARGIN };
                
                #define CEU_TAG_TITLE (47)
                CEU_Tags_Names ceu_tag_TITLE = { CEU_TAG_TITLE, ":TITLE", &ceu_tag_GAP };
                
                CEU_Tags_Names* CEU_TAGS = &ceu_tag_TITLE;
            
    void ceux_dump (CEU_Stack* S, int n) {
        printf(">>> DUMP | n=%d | S=%p\n", S->n, S);
        for (int i=n; i<S->n; i++) {
            printf(">>> [%d]: [%d] ", i, ceux_peek(S,i).type);
            ceu_print1(ceux_peek(S,i));
            puts("");
        }
    }
    int ceux_n_get (CEU_Stack* S) {
        return S->n;
    }
    int ceux_push (CEU_Stack* S, int inc, CEU_Value v) {
        assert(S->n<CEU_STACK_MAX && "TODO: stack error");
        if (inc) {
            ceu_gc_inc_val(v);
        }
        S->buf[S->n++] = v;
        return S->n-1;
    }
    CEU_Value ceux_pop (CEU_Stack* S, int dec) {
        assert(S->n>0 && "TODO: stack error");
        CEU_Value v = S->buf[--S->n];
        if (dec) {
            ceu_gc_dec_val(v);
        }
        return v;
    }
    void ceux_pop_n (CEU_Stack* S, int n) {
        for (int i=0; i<n; i++) {
            ceux_pop(S, 1);
        }
    }
    CEU_Value ceux_peek (CEU_Stack* S, int i) {
        assert(i>=0 && i<S->n && "TODO: stack error");
        return S->buf[i];
    }
    void ceux_drop (CEU_Stack* S, int n) {
        assert(n<=S->n && "BUG: index out of range");
        for (int i=0; i<n; i++) {
            ceu_gc_dec_val(S->buf[--S->n]);
        }
    }
    void ceux_n_set (CEU_Stack* S, int n) {
        assert(n>=0 && n<=S->n && "TODO: stack error");
        for (int i=S->n; i>n; i--) {
            ceu_gc_dec_val(S->buf[--S->n]);
        }
    }
    void ceux_repl (CEU_Stack* S, int i, CEU_Value v) {
        assert(i>=0 && i<S->n && "TODO: stack error");
        ceu_gc_inc_val(v);
        ceu_gc_dec_val(S->buf[i]);
        S->buf[i] = v;
    }
    void ceux_dup (CEU_Stack* S, int i) {
        ceux_push(S, 1, ceux_peek(S,i));
    }
    void ceux_dup_n (CEU_Stack* S, int i, int n) {
        for (int x=i; x<i+n; x++) {
            ceux_dup(S, x);
        }
    }
    void ceux_copy (CEU_Stack* S, int i, int j) {
        assert(i>=0 && i<S->n && "TODO: stack error");
        assert(j>=0 && j<S->n && "TODO: stack error");
        assert(i!=j && "TODO: invalid move");
        ceu_gc_dec_val(S->buf[i]);
        S->buf[i] = S->buf[j];
        ceu_gc_inc_val(S->buf[i]);
    }
    void ceux_move (CEU_Stack* S, int i, int j) {
        assert(i>=0 && i<S->n && "TODO: stack error");
        assert(j>=0 && j<S->n && "TODO: stack error");
        if (i == j) {
            // nothing to change
        } else {
            ceu_gc_dec_val(S->buf[i]);
            S->buf[i] = S->buf[j];
            S->buf[j] = (CEU_Value) { CEU_VALUE_NIL };
        }
    }

    void ceux_ins (CEU_Stack* S, int i, CEU_Value v) {
        // [...,x,...]
        //      ^ i
        assert(i>=0 && i<=S->n && "TODO: stack error");
        for (int j=S->n; j>i; j--) {
            S->buf[j] = S->buf[j-1];
        }
        ceu_gc_inc_val(v);
        S->buf[i] = v;
        S->n++;
        // [...,nil,x,...]
        //       ^ i
    }
    
    void ceux_rem (CEU_Stack* S, int i) {
        // [pre,x,pos]
        //      ^ i
        assert(i>=0 && i<S->n && "TODO: stack error");
        ceu_gc_dec_val(S->buf[i]);
        for (int j=i; j<S->n-1; j++) {
            S->buf[j] = S->buf[j+1];
        }
        S->n--;
        // [pre,pos]
    }
    
    // ceux_block_*
    //  - needs to clear locals on enter and leave
    //  - enter: initialize all vars to nil (prevents garbage)
    //  - leave: gc locals
    
    void ceux_block_enter (CEU_Stack* S, int base, int n CEU4(COMMA CEU_Exe* exe)) {
        // clear locals
        // TODO: use memset=0
        for (int i=0; i<n; i++) {
            ceux_repl(S, base+i, (CEU_Value) { CEU_VALUE_NIL });
        }
    #if CEU >= 4
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_BLOCK, {.Block=NULL} });
    #else
        ceux_push(S, 1, (CEU_Value) { CEU_VALUE_BLOCK });
    #endif
    }
    
    void ceux_block_leave (CEU_Stack* S, int base, int n, int out) {
        int I = -1;
        for (int i=S->n-1; i>=0; i--) {
            CEU_Value blk = ceux_peek(S,i);
            if (blk.type == CEU_VALUE_BLOCK) {
    #if CEU >= 4
                if (blk.Block != NULL) {
                    CEU_LNKS(blk.Block)->up.blk = NULL; // also on ceu_task_unlink (if unlinked before leave)
                }

                {
                    CEU_Block cur = blk.Block;
                    while (cur != NULL) {
                        int term = (CEU5(cur->Any.type==CEU_VALUE_EXE_TASK &&) cur->Exe_Task.status==CEU_EXE_STATUS_TERMINATED);
                        ceu_abort_dyn(cur);
                        CEU_Dyn* nxt = CEU_LNKS(cur)->sd.nxt;
                        if (!term) {
                            ceu_gc_dec_dyn(cur); // TODO: could affect nxt?
                        }
                        cur = nxt;
                    }
                }
    #endif
                I = i;
                break;
            }
        }
        assert(I >= 0);
        
        // clear locals after clear block
        // TODO: use memset=0
        for (int i=n-1; i>=0; i--) {
            ceux_repl(S, base+i, (CEU_Value) { CEU_VALUE_NIL });
        }

    #if CEU >= 2
        // in case of error, out must be readjusted to the error stack:
        // [BLOCK,...,n,pay,err]
        //  - ... - error messages
        //  - n   - number of error messages
        //  - pay - error payload
        //  - err - error value
        if (CEU_ERROR_IS(S)) {
            CEU_Value n = ceux_peek(S,SS(-3));
            assert(n.type == CEU_VALUE_NUMBER);
            out = n.Number + 1 + 1 + 1;
        }
    #endif

        for (int i=0; i<out; i++) {
            ceux_move(S, I+i, SS(-out+i));
        }
        ceux_n_set(S, I+out);
    }
    
    int ceux_call_pre (CEU_Stack* S, CEU_Clo* clo, int* inp) {
        // fill missing args with nils
        {
            int N = clo->args - *inp;
            for (int i=0; i<N; i++) {
                ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NIL });
                (*inp)++;
            }
        }
        
        int base = S->n;

        // [clo,args,?]
        //           ^ base

        // place upvs+locs
        {
            for (int i=0; i<clo->upvs.its; i++) {
                ceux_push(S, 1, clo->upvs.buf[i]);
            }
            for (int i=0; i<clo->locs; i++) {
                ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NIL });
            }
        }
        // [clo,args,upvs,locs]
        //           ^ base

        return base;
    }
    
    int ceux_call_pos (CEU_Stack* S, int ret, int* out) {
        // in case of error, out must be readjusted to the error stack:
        // [clo,args,upvs,locs,...,n,pay,err]
        //  - ... - error messages
        //  - n   - number of error messages
        //  - pay - error payload
        //  - err - error value
        if (ret>0 && CEU_ERROR_IS(S)) {
            CEU_Value n = ceux_peek(S,SS(-3));
            assert(n.type == CEU_VALUE_NUMBER);
            *out = n.Number + 1 + 1 + 1;
            return 1;
        }
         
        if (*out == CEU_MULTI) {     // any rets is ok
            *out = ret;
        } else if (ret < *out) {     // less rets than requested
           // fill rets up to outs
            for (int i=0; i<*out-ret; i++) {
                ceux_push(S, 1, (CEU_Value) { CEU_VALUE_NIL });
            }
        } else if (ret > *out) {     // more rets than requested
            ceux_pop_n(S, ret-*out);
        } else { // ret == out      // exact rets requested
            // ok
        }
        return 0;
    }
    
    int ceux_call (CEUX* X1, int inp, int out) {
        // [clo,inps]
        CEU_Value clo = ceux_peek(X1->S, XX1(-inp-1));
        if (clo.type != CEU_VALUE_CLO_FUNC) {
            return ceu_error_s(X1->S, "call error : expected function");
        }

        int base = ceux_call_pre(X1->S, &clo.Dyn->Clo, &inp);

        // [clo,args,upvs,locs]
        //           ^ base

        CEUX X2 = { X1->S, base, inp CEU3(COMMA CEU_ACTION_CALL COMMA {.exe=X1->exe}) CEU4(COMMA X1->now COMMA X1) };
        int ret = clo.Dyn->Clo.proto(&X2);
        
        // [clo,args,upvs,locs,rets]
        //           ^ base
        
        ceux_call_pos(X1->S, ret, &out);        
        
        // [clo,args,upvs,locs,out]
        //           ^ base
        
        // move rets to begin, replacing [clo,args,upvs,locs]
        {
            for (int i=0; i<out; i++) {
                ceux_move(X1->S, base-inp-1+i, X1->S->n-out+i);
            }

            // [outs,x,x,x,x]
            //           ^ base
            ceux_n_set(X1->S, base-inp-1+out);
        }
        // [outs]
        //      ^ base
        
        return out;
    }
    
#if CEU >= 3
    int ceux_resume (CEUX* X1, int inp, int out, CEU_ACTION act CEU4(COMMA uint32_t now)) {
        // X1: [exe,inps]
        //assert((inp<=1 || CEU_ERROR_IS(X1->S)) && "TODO: varargs resume");

        CEU_Value exe = ceux_peek(X1->S, XX1(-inp-1));
        if (!(ceu_isexe_val(exe) && (exe.Dyn->Exe.status==CEU_EXE_STATUS_YIELDED || act==CEU_ACTION_ABORT))) {
            return ceu_error_s(X1->S, "resume error : expected yielded coro");
        }
        assert(exe.Dyn->Exe.clo.type==CEU_VALUE_CLO_CORO CEU4(|| exe.Dyn->Exe_Task.clo.type==CEU_VALUE_CLO_TASK));
        CEU_Clo* clo = &exe.Dyn->Exe.clo.Dyn->Clo;
        
        // X1: [exe,inps]
        // X2: [...]
        CEUX* X2 = exe.Dyn->Exe.X;
        
    #if CEU >= 4
        X2->up = X1;
    #endif
        
        {
            int n = XX1(-inp);
            for (int i=n; i<n+inp; i++) {
                ceux_push(X2->S, 1, ceux_peek(X1->S,i));
            }
        }
        
        ceu_gc_inc_val(exe);
        ceux_n_set(X1->S, XX1(-inp-1));
        // X1: []
        // X2: [...,inps]
        
        // first resume: place upvs+locs
        if (exe.Dyn->Exe.pc == 0) {
            X2->base = ceux_call_pre(X2->S, clo, &inp);
            X2->args = inp;
            // X2: [args,upvs,locs]
            //           ^ base
        } else {
            //X2->base = <already set>
            // X2: [args,upvs,locs,...,inps]
            //           ^ base
        }
        X2->action = act;
    #if CEU >= 4
        X2->now = now;
    #endif

        int ret = clo->proto(X2);
        
        // X2: [args,upvs,locs,...,rets]
        
        int err = ceux_call_pos(X2->S, ret, &out);        
        
        // X1: []
        // X2: [args,upvs,locs,...,outs]

        for (int i=0; i<out; i++) {
            ceux_push(X1->S, 1, ceux_peek(X2->S,XX2(-out)+i));                               
        }
        if (err) {
            ceux_n_set(X2->S, 0);
        } else {
            ceux_n_set(X2->S, XX2(-out));
        }
        
        // X1: [outs]
        // X2: []
        
        ceu_gc_dec_val(exe);
        return out;
    }
#endif

#if CEU >= 4
    CEU_Block* ceu_up_blk (CEU_Stack* S) {
        for (int i = S->n-1; i>=0; i--) {
            CEU_Value v = ceux_peek(S, i);
            if (v.type == CEU_VALUE_BLOCK) {
                return &S->buf[i].Block;
            }
        }
        return NULL; //assert(0 && "bug found: no block found");
    }

    CEU_Exe_Task* ceu_up_tsk (CEUX* X) {
        if (X->exe!=NULL && X->exe->type==CEU_VALUE_EXE_TASK) {
            return (CEU_Exe_Task*) X->exe;
        } else if (X->up == NULL) {
            return &CEU_GLOBAL_TASK;
        } else {
            return ceu_up_tsk(X->up);
        }
    }

    int ceux_spawn (CEUX* X1, int inp, uint8_t now) {
        // X1: [tsks,clo,inps]

        #if CEU >= 5
        CEU_Value up_tsks = ceux_peek(X1->S, XX1(-inp-2));
        if (up_tsks.type!=CEU_VALUE_NIL && up_tsks.type!=CEU_VALUE_TASKS) {
            return ceu_error_s(X1->S, "spawn error : invalid pool");
        }
        CEU_Tasks* xup_tsks = (up_tsks.type == CEU_VALUE_NIL) ? NULL : &up_tsks.Dyn->Tasks;
        #endif

        CEU_Value clo = ceux_peek(X1->S, XX1(-inp-1));
        if (clo.type != CEU_VALUE_CLO_TASK) {
            return ceu_error_s(X1->S, "spawn error : expected task");
        }
        
        CEU_Value exe; {
        #if CEU >= 5
            if (xup_tsks != NULL) {
                exe = ceu_create_exe_task(clo, (CEU_Dyn*) xup_tsks, NULL);
            } else {
                exe = ceu_create_exe_task(clo, (CEU_Dyn*) ceu_up_tsk(X1), ceu_up_blk(X1->S));
            }
        #else
            exe = ceu_create_exe_task(clo, (CEU_Dyn*) ceu_up_tsk(X1), ceu_up_blk(X1->S));
        #endif
        }
        if (exe.type == CEU_VALUE_ERROR) {
            return ceu_error_e(X1->S, exe);
        }
    #if CEU >= 5
        else if (exe.type == CEU_VALUE_NIL) {
            // X1: [tsks,clo,inps]
            ceux_pop_n(X1->S, 2+inp);
            ceux_push(X1->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            // X1: [nil]
            return 1;
        }
    #endif
        assert(exe.Dyn->Exe_Task.clo.type == CEU_VALUE_CLO_TASK);
        
        ceux_repl(X1->S, XX1(-inp-1), exe);
        // X1: [tsks,exe,inps]
        
        ceu_gc_inc_val(exe);    // keep exe alive to return it  
        int ret = ceux_resume(X1, inp, 0, CEU_ACTION_RESUME CEU4(COMMA now));
        // X1: [tsks]
        
        if (ret > 0) {
            // error
        } else {
            ret = 1;
    #if CEU >= 5
            ceux_pop(X1->S, 1); // [tsks]
    #endif
            ceux_push(X1->S, 1, exe);        // returns exe to caller
            // X1: [exe]
        }
        ceu_gc_dec_val(exe);    // dec after push above
        
        return ret;
    }
#endif
    
    CEU_Value ceu_dyn_to_val (CEU_Dyn* dyn) {
        return (CEU_Value) { dyn->Any.type, {.Dyn=dyn} };
    }
    
    int ceu_dump_f (CEUX* X) {
        assert(X->args == 1);
    #ifdef CEU_DEBUG
        ceu_dump_val(ceux_peek(X->S, ceux_arg(X,0)));
        return 0;
    #else
        return ceu_error_s(X->S, "debug is off");
    #endif
    }

    int ceu_as_bool (CEU_Value v) {
        return !(v.type==CEU_VALUE_NIL || (v.type==CEU_VALUE_BOOL && !v.Bool));
    }
    int ceu_type_f (CEUX* X) {
        assert(X->args==1 && "bug found");
        int type = ceux_peek(X->S, ceux_arg(X,0)).type;
        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=type} });
        return 1;
    }
    
    CEU_Value _ceu_sup_ (CEU_Value sup, CEU_Value sub) {
        assert(sup.type == CEU_VALUE_TAG);
        assert(sub.type == CEU_VALUE_TAG);
        
        //printf("sup=0x%08X vs sub=0x%08X\n", sup->Tag, sub->Tag);
        int sup0 = sup.Tag & 0x000000FF;
        int sup1 = sup.Tag & 0x0000FF00;
        int sup2 = sup.Tag & 0x00FF0000;
        int sup3 = sup.Tag & 0xFF000000;
        int sub0 = sub.Tag & 0x000000FF;
        int sub1 = sub.Tag & 0x0000FF00;
        int sub2 = sub.Tag & 0x00FF0000;
        int sub3 = sub.Tag & 0xFF000000;

        return (CEU_Value) { CEU_VALUE_BOOL, { .Bool =
            (sup0 == sub0) && ((sup1 == 0) || (
                (sup1 == sub1) && ((sup2 == 0) || (
                    (sup2 == sub2) && ((sup3 == 0) || (
                        (sup3 == sub3)
                    ))
                ))
            ))
        } };
    }
    int ceu_sup_question__f (CEUX* X) {
        assert(X->args >= 2);
        CEU_Value sup = ceux_peek(X->S, ceux_arg(X,0));
        CEU_Value sub = ceux_peek(X->S, ceux_arg(X,1));
        CEU_Value ret = _ceu_sup_(sup, sub);
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
    CEU_Value _ceu_tags_all_ (CEU_Value dyn) {
        int len = 0; {
            CEU_Tags_List* cur = dyn.Dyn->Any.tags;
            while (cur != NULL) {
                len++;
                cur = cur->next;
            }
        }
        CEU_Value tup = ceu_create_tuple(len);
        {
            CEU_Tags_List* cur = dyn.Dyn->Any.tags;
            int i = 0;
            while (cur != NULL) {
                ceu_tuple_set(&tup.Dyn->Tuple, i++, (CEU_Value) { CEU_VALUE_TAG, {.Tag=cur->tag} });
                cur = cur->next;
            }
        }
        return tup;
    }
    
    CEU_Value ceu_tags_chk (CEU_Value dyn, CEU_Value tag) {
        CEU_Value ret = { CEU_VALUE_BOOL, {.Bool=0} };
        CEU_Tags_List* cur = (dyn.type < CEU_VALUE_DYNAMIC) ? NULL : dyn.Dyn->Any.tags;
        while (cur != NULL) {
            CEU_Value sub = { CEU_VALUE_TAG, {.Tag=cur->tag} };
            ret = _ceu_sup_(tag, sub);
            if (ret.Bool) {
                break;
            }
            cur = cur->next;
        }
        return ret;
    }
        
    void ceu_tags_set (CEU_Value dyn, CEU_Value tag, int on) {
        assert(dyn.type > CEU_VALUE_DYNAMIC);
        if (on) {   // add
            CEU_Value has = ceu_tags_chk(dyn, tag);
            if (!has.Bool) {
                CEU_Tags_List* v = malloc(sizeof(CEU_Tags_List));
                assert(v != NULL);
                v->tag = tag.Tag;
                v->next = dyn.Dyn->Any.tags;
                dyn.Dyn->Any.tags = v;
            }
        } else {            // rem
            CEU_Tags_List** cur = &dyn.Dyn->Any.tags;
            while (*cur != NULL) {
                if ((*cur)->tag == tag.Tag) {
                    CEU_Tags_List* v = *cur;
                    *cur = v->next;
                    free(v);
                    break;
                }
                cur = &(*cur)->next;
            }
        }
    }
        
    int ceu_tags_f (CEUX* X) {
        assert(X->args >= 1);
        CEU_Value dyn = ceux_peek(X->S, ceux_arg(X,0));
        CEU_Value tag; // = (CEU_Value) { CEU_VALUE_NIL };
        if (X->args >= 2) {
            tag = ceux_peek(X->S, ceux_arg(X,1));
            assert(tag.type == CEU_VALUE_TAG);
        }
        
        switch (X->args) {
            case 1: {   // all tags
                CEU_Value ret = _ceu_tags_all_(dyn);
                ceux_push(X->S, 1, ret);
                break;
            }
            case 2: {   // check tag
                CEU_Value ret = ceu_tags_chk(dyn, tag);
                ceux_push(X->S, 1, ret);
                break;
            }
            default: {   // add/rem tag
                CEU_Value bool = ceux_peek(X->S, ceux_arg(X,2));
                assert(bool.type == CEU_VALUE_BOOL);
                ceu_tags_set(dyn, tag, bool.Bool);
                ceux_dup(X->S, ceux_arg(X,0));  // keep dyn
                break;
            }
        }
        return 1;
    }
    
    // TO-TAG-*

    int ceu_to_dash_tag_dash_string_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value str = ceux_peek(X->S, ceux_arg(X,0));
        assert(str.type==CEU_VALUE_VECTOR && str.Dyn->Vector.unit==CEU_VALUE_CHAR);
        CEU_Tags_Names* cur = CEU_TAGS;
        CEU_Value ret = (CEU_Value) { CEU_VALUE_NIL };
        while (cur != NULL) {
            if (!strcmp(cur->name,str.Dyn->Vector.buf)) {
                ret = (CEU_Value) { CEU_VALUE_TAG, {.Tag=cur->tag} };
                break;
            }
            cur = cur->next;
        }
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
    // TO-STRING-*

    char* ceu_to_dash_string_dash_tag (int tag) {
        CEU_Tags_Names* cur = CEU_TAGS;
        while (cur != NULL) {
            if (cur->tag == tag) {
                return cur->name;
            }
            cur = cur->next;
        }
        assert(0 && "bug found");
    }
    
    CEU_Value ceu_to_dash_string_dash_pointer (const char* ptr) {
        assert(ptr != NULL);
        CEU_Value str = ceu_create_vector();
        int len = strlen(ptr);
        for (int i=0; i<len; i++) {
            CEU_Value chr = { CEU_VALUE_CHAR, {.Char=ptr[i]} };
            ceu_vector_set(&str.Dyn->Vector, i, chr);
        }
        return str;
    }
    
    int ceu_to_dash_string_dash_pointer_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value ptr = ceux_peek(X->S, ceux_arg(X,0));
        assert(ptr.type==CEU_VALUE_POINTER && ptr.Pointer!=NULL);
        ceux_push(X->S, 1, ceu_to_dash_string_dash_pointer(ptr.Pointer));
        return 1;
    }

    int ceu_to_dash_string_dash_tag_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value t = ceux_peek(X->S, ceux_arg(X,0));
        assert(t.type == CEU_VALUE_TAG);        
        ceux_push(X->S, 1, ceu_to_dash_string_dash_pointer(ceu_to_dash_string_dash_tag(t.Tag)));
        return 1;
    }

    int ceu_to_dash_string_dash_number_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value n = ceux_peek(X->S, ceux_arg(X,0));
        assert(n.type == CEU_VALUE_NUMBER);
        
        char str[255];
        snprintf(str, 255, "%g", n.Number);
        assert(strlen(str) < 255);

        ceux_push(X->S, 1, ceu_to_dash_string_dash_pointer(str));
        return 1;
    }
    
    #define ceu_sizeof(type, member) sizeof(((type *)0)->member)
    int ceu_type_to_size (int type) {
        switch (type) {
            case CEU_VALUE_NIL:
            case CEU_VALUE_ERROR:
                return 0;
            case CEU_VALUE_TAG:
                return ceu_sizeof(CEU_Value, Tag);
            case CEU_VALUE_BOOL:
                return ceu_sizeof(CEU_Value, Bool);
            case CEU_VALUE_CHAR:
                return ceu_sizeof(CEU_Value, Char);
            case CEU_VALUE_NUMBER:
                return ceu_sizeof(CEU_Value, Number);
            case CEU_VALUE_POINTER:
                return ceu_sizeof(CEU_Value, Pointer);
            default:
                return ceu_sizeof(CEU_Value, Dyn);
        }
    }
    
    CEU_Value ceu_col_get (CEU_Value col, CEU_Value key) {
        CEU_Value err = ceu_col_check(col,key);
        if (err.type == CEU_VALUE_ERROR) {
            return err;
        }
        switch (col.type) {
            case CEU_VALUE_TUPLE:
                return col.Dyn->Tuple.buf[(int) key.Number];
            case CEU_VALUE_VECTOR:
                return ceu_vector_get(&col.Dyn->Vector, key.Number);
                break;
            case CEU_VALUE_DICT:
                return ceu_dict_get(&col.Dyn->Dict, key);
            default:
                assert(0 && "bug found");
        }
    }
    
    CEU_Value ceu_col_set (CEU_Value col, CEU_Value key, CEU_Value val) {
        CEU_Value ok = { CEU_VALUE_NIL };
        switch (col.type) {
            case CEU_VALUE_TUPLE:
                ceu_tuple_set(&col.Dyn->Tuple, key.Number, val);
                break;
            case CEU_VALUE_VECTOR:
                ceu_vector_set(&col.Dyn->Vector, key.Number, val);
                break;
            case CEU_VALUE_DICT: {
                ok = ceu_dict_set(&col.Dyn->Dict, key, val);
                break;
            }
            default:
                assert(0 && "bug found");
        }
        return ok;
    }
    
    void ceu_tuple_set (CEU_Tuple* tup, int i, CEU_Value v) {
        ceu_gc_inc_val(v);
        ceu_gc_dec_val(tup->buf[i]);
        tup->buf[i] = v;
    }
    
    CEU_Value ceu_vector_get (CEU_Vector* vec, int i) {
        if (i<0 || i>=vec->its) {
            return (CEU_Value) { CEU_VALUE_ERROR, {.Error="index error : out of bounds"} };
        }
        int sz = ceu_type_to_size(vec->unit);
        CEU_Value ret = (CEU_Value) { vec->unit };
        memcpy(&ret.Number, vec->buf+i*sz, sz);
        return ret;
    }
    
    void ceu_vector_set (CEU_Vector* vec, int i, CEU_Value v) {
        if (v.type == CEU_VALUE_NIL) {           // pop
            assert(i == vec->its-1);
            CEU_Value ret = ceu_vector_get(vec, i);
            assert(ret.type != CEU_VALUE_ERROR);
            ceu_gc_dec_val(ret);
            vec->its--;
        } else {
            if (vec->its == 0) {
                vec->unit = v.type;
            } else {
                assert(v.type == vec->unit);
            }
            int sz = ceu_type_to_size(vec->unit);
            if (i == vec->its) {           // push
                if (i == vec->max) {
                    vec->max = vec->max*2 + 1;    // +1 if max=0
                    vec->buf = realloc(vec->buf, vec->max*sz + 1);
                    assert(vec->buf != NULL);
                }
                ceu_gc_inc_val(v);
                vec->its++;
                vec->buf[sz*vec->its] = '\0';
            } else {                            // set
                CEU_Value ret = ceu_vector_get(vec, i);
                assert(ret.type != CEU_VALUE_ERROR);
                ceu_gc_inc_val(v);
                ceu_gc_dec_val(ret);
                assert(i < vec->its);
            }
            memcpy(vec->buf + i*sz, (char*)&v.Number, sz);
        }
    }
    
    CEU_Value ceu_vector_from_c_string (const char* str) {
        CEU_Value vec = ceu_create_vector();
        int N = strlen(str);
        for (int i=0; i<N; i++) {
            ceu_vector_set(&vec.Dyn->Vector, vec.Dyn->Vector.its, (CEU_Value) { CEU_VALUE_CHAR, {.Char=str[i]} });
        }
        return vec;
    }

    int ceu_next_dash_dict_f (CEUX* X) {
        assert(X->args==1 || X->args==2);
        CEU_Value dict = ceux_peek(X->S, ceux_arg(X,0));
        CEU_Value ret;
        if (dict.type != CEU_VALUE_DICT) {
            return ceu_error_s(X->S, "next-dict error : expected dict");
        } else {
            CEU_Value key = (X->args == 1) ? ((CEU_Value) { CEU_VALUE_NIL }) : ceux_peek(X->S, ceux_arg(X,1));
            if (key.type == CEU_VALUE_NIL) {
                ret = (*dict.Dyn->Dict.buf)[0][0];
            } else {
                ret = (CEU_Value) { CEU_VALUE_NIL };
                for (int i=0; i<dict.Dyn->Dict.max-1; i++) {     // -1: last element has no next
                    CEU_Value ok = _ceu_equals_equals_(key, (*dict.Dyn->Dict.buf)[i][0]);
                    assert(ok.type != CEU_VALUE_ERROR);
                    if (ok.Bool) {
                        ret = (*dict.Dyn->Dict.buf)[i+1][0];
                        break;
                    }
                }
            }
        }
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
    int ceu_dict_key_to_index (CEU_Dict* col, CEU_Value key, int* idx) {
        *idx = -1;
        for (int i=0; i<col->max; i++) {
            CEU_Value cur = (*col->buf)[i][0];
            CEU_Value ret = _ceu_equals_equals_(key, cur);
            assert(ret.type != CEU_VALUE_ERROR);
            if (ret.Bool) {
                *idx = i;
                return 1;
            } else {
                if (*idx==-1 && cur.type==CEU_VALUE_NIL) {
                    *idx = i;
                }
            }
        }
        return 0;
    }        
    CEU_Value ceu_dict_get (CEU_Dict* col, CEU_Value key) {
        int i;
        int ok = ceu_dict_key_to_index(col, key, &i);
        if (ok) {
            return (*col->buf)[i][1];
        } else {
            return (CEU_Value) { CEU_VALUE_NIL };
        }
    }        
    CEU_Value ceu_dict_set (CEU_Dict* col, CEU_Value key, CEU_Value val) {
        if (key.type == CEU_VALUE_NIL) {
            return (CEU_Value) { CEU_VALUE_ERROR, {.Error="dict error : index cannot be nil"} };
        }
        int old;
        ceu_dict_key_to_index(col, key, &old);
        if (old == -1) {
            old = col->max;
            int new = MAX(5, old * 2);
            col->max = new;
            col->buf = realloc(col->buf, new*2*sizeof(CEU_Value));
            assert(col->buf != NULL);
            memset(&(*col->buf)[old], 0, (new-old)*2*sizeof(CEU_Value));  // x[i]=nil
        }
        assert(old != -1);
        
        CEU_Value vv = ceu_dict_get(col, key);
        
        if (val.type == CEU_VALUE_NIL) {
            ceu_gc_dec_val(vv);
            ceu_gc_dec_val(key);
            (*col->buf)[old][0] = (CEU_Value) { CEU_VALUE_NIL };
        } else {
            ceu_gc_inc_val(val);
            ceu_gc_dec_val(vv);
            if (vv.type == CEU_VALUE_NIL) {
                ceu_gc_inc_val(key);
            }
            (*col->buf)[old][0] = key;
            (*col->buf)[old][1] = val;
        }
        return (CEU_Value) { CEU_VALUE_NIL };
    }        
    
    CEU_Value ceu_col_check (CEU_Value col, CEU_Value idx) {
        if (col.type<CEU_VALUE_TUPLE || col.type>CEU_VALUE_DICT) {                
            return (CEU_Value) { CEU_VALUE_ERROR, {.Error="index error : expected collection"} };
        }
        if (col.type != CEU_VALUE_DICT) {
            if (idx.type != CEU_VALUE_NUMBER) {
                return (CEU_Value) { CEU_VALUE_ERROR, {.Error="index error : expected number"} };
            }
            if (col.type==CEU_VALUE_TUPLE && (idx.Number<0 || idx.Number>=col.Dyn->Tuple.its)) {                
                return (CEU_Value) { CEU_VALUE_ERROR, {.Error="index error : out of bounds"} };
            }
            if (col.type==CEU_VALUE_VECTOR && (idx.Number<0 || idx.Number>col.Dyn->Vector.its)) {                
                return (CEU_Value) { CEU_VALUE_ERROR, {.Error="index error : out of bounds"} };
            }
        }
        return (CEU_Value) { CEU_VALUE_NIL };
    }
    
    CEU_Value ceu_create_tuple (int n) {
        ceu_debug_add(CEU_VALUE_TUPLE);
        CEU_Tuple* ret = malloc(sizeof(CEU_Tuple) + n*sizeof(CEU_Value));
        assert(ret != NULL);
        *ret = (CEU_Tuple) {
            CEU_VALUE_TUPLE, 0, NULL,
            n, {}
        };
        memset(ret->buf, 0, n*sizeof(CEU_Value));
        return (CEU_Value) { CEU_VALUE_TUPLE, {.Dyn=(CEU_Dyn*)ret} };
    }
    
    int ceu_tuple_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value arg = ceux_peek(X->S, ceux_arg(X,0));
        assert(arg.type == CEU_VALUE_NUMBER);
        CEU_Value ret = ceu_create_tuple(arg.Number);
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
    CEU_Value ceu_create_vector (void) {
        ceu_debug_add(CEU_VALUE_VECTOR);
        CEU_Vector* ret = malloc(sizeof(CEU_Vector));
        assert(ret != NULL);
        char* buf = malloc(1);  // because of '\0' in empty strings
        assert(buf != NULL);
        buf[0] = '\0';
        *ret = (CEU_Vector) {
            CEU_VALUE_VECTOR, 0,  NULL,
            0, 0, CEU_VALUE_NIL, buf
        };
        return (CEU_Value) { CEU_VALUE_VECTOR, {.Dyn=(CEU_Dyn*)ret} };
    }
    
    CEU_Value ceu_create_dict (void) {
        ceu_debug_add(CEU_VALUE_DICT);
        CEU_Dict* ret = malloc(sizeof(CEU_Dict));
        assert(ret != NULL);
        *ret = (CEU_Dict) {
            CEU_VALUE_DICT, 0, NULL,
            0, NULL
        };
        return (CEU_Value) { CEU_VALUE_DICT, {.Dyn=(CEU_Dyn*)ret} };
    }
    
    CEU_Value ceu_create_clo (CEU_VALUE type, CEU_Proto proto, int args, int locs, int upvs) {
        ceu_debug_add(type);
        CEU_Clo* ret = malloc(CEU4(type==CEU_VALUE_CLO_TASK ? sizeof(CEU_Clo_Task) :) sizeof(CEU_Clo));
        assert(ret != NULL);
        CEU_Value* buf = malloc(upvs * sizeof(CEU_Value));
        assert(buf != NULL);
        for (int i=0; i<upvs; i++) {
            buf[i] = (CEU_Value) { CEU_VALUE_NIL };
        }
        *ret = (CEU_Clo) {
            type, 0, NULL,
            proto,
            args, locs, { upvs, buf }
        };
        return (CEU_Value) { type, {.Dyn=(CEU_Dyn*)ret } };
    }

    #if CEU >= 4
    CEU_Value ceu_create_clo_task (CEU_Proto proto, int args, int locs, int upvs, CEU_Exe_Task* up_tsk) {
        CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_TASK, proto, args, locs, upvs);
        assert(clo.type == CEU_VALUE_CLO_TASK);
        clo.Dyn->Clo_Task.up_tsk = up_tsk;
        return clo;
    }
    #endif
    
    #if CEU >= 3
    CEU_Value ceu_create_exe (int type, int sz, CEU_Value clo) {
        ceu_debug_add(type);
        assert(clo.type==CEU_VALUE_CLO_CORO CEU4(|| clo.type==CEU_VALUE_CLO_TASK));
        ceu_gc_inc_val(clo);
        
        CEU_Exe* ret = malloc(sz);
        assert(ret != NULL);
        CEUX* X = malloc(sizeof(CEUX));
        CEU_Stack* S = malloc(sizeof(CEU_Stack));
        assert(X!=NULL && S!=NULL);
        S->n = 0;
        //S->buf = <dynamic>    // TODO
        *X = (CEUX) { S, -1, -1, CEU_ACTION_INVALID, {.exe=ret} CEU4(COMMA CEU_TIME-1 COMMA NULL) };
            // X->up is set on resume, not here on creation

        *ret = (CEU_Exe) {
            type, 0, NULL,
            CEU_EXE_STATUS_YIELDED, clo, 0, X
        };
        
        return (CEU_Value) { type, {.Dyn=(CEU_Dyn*)ret } };
    }
    #endif
    
    #if CEU >= 4
    CEU_Value ceu_create_exe_task (CEU_Value clo, CEU_Dyn* up_dyn, CEU_Block* up_blk) {
    #if CEU >= 5
        int ceu_tasks_n (CEU_Tasks* tsks) {
            int n = 0;
            CEU_Exe_Task* cur = (CEU_Exe_Task*) tsks->lnks.dn.fst;
            while (cur != NULL) {
                n++;
                cur = (CEU_Exe_Task*) cur->lnks.sd.nxt;
            }
            return n;
        }
        if (!ceu_isexe_dyn(up_dyn)) {
            CEU_Tasks* tsks = (CEU_Tasks*) up_dyn;
            if (tsks->max!=0 && ceu_tasks_n(tsks)>=tsks->max) {
                return (CEU_Value) { CEU_VALUE_NIL };
            }
        }
    #endif
        
        if (clo.type != CEU_VALUE_CLO_TASK) {
            return (CEU_Value) { CEU_VALUE_ERROR, {.Error="spawn error : expected task"} };
        }

        CEU_Value ret = ceu_create_exe(CEU_VALUE_EXE_TASK, sizeof(CEU_Exe_Task), clo);
        CEU_Exe_Task* dyn = &ret.Dyn->Exe_Task;
        
        ceu_gc_inc_dyn((CEU_Dyn*) dyn);    // up_blk/tsks holds a strong reference

        dyn->time = CEU_TIME;
        dyn->pub = (CEU_Value) { CEU_VALUE_NIL };

        dyn->lnks = (CEU_Links) { {up_dyn,NULL}, {NULL,NULL}, {NULL,NULL} };

        if (CEU5(dyn!=NULL && ceu_isexe_dyn(up_dyn) &&) *up_blk==NULL) {
            dyn->lnks.up.blk = up_blk;    // only the first task points up
            *up_blk = (CEU_Dyn*) dyn;
        }
        
        if (up_dyn != NULL) {
            CEU_Links* up_lnks = CEU_LNKS(up_dyn);        
            if (up_lnks->dn.fst == NULL) {
                assert(up_lnks->dn.lst == NULL);
                up_lnks->dn.fst = (CEU_Dyn*) dyn;
            } else if (up_lnks->dn.lst != NULL) {
                CEU_LNKS(up_lnks->dn.lst)->sd.nxt = (CEU_Dyn*) dyn;
                dyn->lnks.sd.prv = up_lnks->dn.lst;
            }
            up_lnks->dn.lst = (CEU_Dyn*) dyn;
        }

        return ret;
    }
    #endif
    
    #if CEU >= 5
    CEU_Value ceu_create_tasks (int max, CEU_Exe_Task* up_tsk, CEU_Block* up_blk) {
        CEU_Tasks* ret = malloc(sizeof(CEU_Tasks));
        assert(ret != NULL);

        *ret = (CEU_Tasks) {
            CEU_VALUE_TASKS, 0, NULL,
            max, { {(CEU_Dyn*)up_tsk,NULL}, {NULL,NULL}, {NULL,NULL} }
        };
        
        ceu_gc_inc_dyn((CEU_Dyn*) ret);    // up_blk/tsks holds a strong reference

        {
            if (*up_blk == NULL) {
                ret->lnks.up.blk = up_blk;    // only the first task points up
                *up_blk = (CEU_Dyn*) ret;
            }
            if (up_tsk->lnks.dn.fst == NULL) {
                assert(up_tsk->lnks.dn.lst == NULL);
                up_tsk->lnks.dn.fst = (CEU_Dyn*) ret;
            } else if (up_tsk->lnks.dn.lst != NULL) {
                CEU_LNKS(up_tsk->lnks.dn.lst)->sd.nxt = (CEU_Dyn*) ret;
                ret->lnks.sd.prv = up_tsk->lnks.dn.lst;
            }
            up_tsk->lnks.dn.lst = (CEU_Dyn*) ret;
        }
        
        return (CEU_Value) { CEU_VALUE_TASKS, {.Dyn=(CEU_Dyn*)ret} };
    }
    #if 0
    CEU_Value ceu_create_track (CEU_Exe_Task* task) {
        ceu_debug_add(CEU_VALUE_TRACK);
        CEU_Track* ret = malloc(sizeof(CEU_Track));
        assert(ret != NULL);
        *ret = (CEU_Track) {
            CEU_VALUE_TRACK, 0, NULL,
            task
        };
        ceu_hold_add((CEU_Dyn*)ret, blk, &blk->dn.dyns);
        return (CEU_Value) { CEU_VALUE_TRACK, {.Dyn=(CEU_Dyn*)ret} };
    }
    #endif
    #endif
    
    void ceu_print1 (CEU_Value v) {
        if (v.type > CEU_VALUE_DYNAMIC) {  // TAGS
            CEU_Value tup = _ceu_tags_all_(v);
            assert(tup.type == CEU_VALUE_TUPLE);
            int N = tup.Dyn->Tuple.its;
            if (N > 0) {
                if (N > 1) {
                    printf("[");
                }
                for (int i=0; i<N; i++) {
                    ceu_print1(tup.Dyn->Tuple.buf[i]);
                    if (i < N-1) {
                        printf(",");
                    }
                }
                if (N > 1) {
                    printf("]");
                }
                printf(" ");
            }
            ceu_gc_free(tup.Dyn);
        }
        switch (v.type) {
            case CEU_VALUE_BLOCK:
    #if CEU >= 4
                printf("block: %p", v.Block);
    #else
                printf("(block sentinel)");
    #endif
                break;
            case CEU_VALUE_NIL:
                printf("nil");
                break;
            case CEU_VALUE_ERROR:
                printf("error: %s", (v.Error==NULL ? "(null)" : v.Error));
                break;
            case CEU_VALUE_TAG:
                printf("%s", ceu_to_dash_string_dash_tag(v.Tag));
                break;
            case CEU_VALUE_BOOL:
                if (v.Bool) {
                    printf("true");
                } else {
                    printf("false");
                }
                break;
            case CEU_VALUE_CHAR:
                putchar(v.Char);
                break;
            case CEU_VALUE_NUMBER:
                printf("%g", v.Number);
                break;
            case CEU_VALUE_POINTER:
                printf("pointer: %p", v.Pointer);
                break;
            case CEU_VALUE_TUPLE:
                printf("[");
                for (int i=0; i<v.Dyn->Tuple.its; i++) {
                    if (i > 0) {
                        printf(",");
                    }
                    ceu_print1(v.Dyn->Tuple.buf[i]);
                }                    
                printf("]");
                break;
            case CEU_VALUE_VECTOR:
                if (v.Dyn->Vector.unit == CEU_VALUE_CHAR) {
                    printf("%s", v.Dyn->Vector.buf);
                } else {
                    printf("#[");
                    for (int i=0; i<v.Dyn->Vector.its; i++) {
                        if (i > 0) {
                            printf(",");
                        }
                        CEU_Value ret = ceu_vector_get(&v.Dyn->Vector, i);
                        assert(ret.type != CEU_VALUE_ERROR);
                        ceu_print1(ret);
                    }                    
                    printf("]");
                }
                break;
            case CEU_VALUE_DICT:
                printf("@[");
                int comma = 0;
                for (int i=0; i<v.Dyn->Dict.max; i++) {
                    if ((*v.Dyn->Dict.buf)[i][0].type != CEU_VALUE_NIL) {
                        if (comma != 0) {
                            printf(",");
                        }
                        comma = 1;
                        printf("(");
                        ceu_print1((*v.Dyn->Dict.buf)[i][0]);
                        printf(",");
                        ceu_print1((*v.Dyn->Dict.buf)[i][1]);
                        printf(")");
                    }
                }                    
                printf("]");
                break;
            case CEU_VALUE_CLO_FUNC:
                printf("func: %p", v.Dyn);
                if (v.Dyn->Clo.upvs.its > 0) {
                    printf(" | [");
                    for (int i=0; i<v.Dyn->Clo.upvs.its; i++) {
                        if (i > 0) {
                            printf(",");
                        }
                        ceu_print1(v.Dyn->Clo.upvs.buf[i]);
                    }
                    printf("]");
                }
                break;
    #if CEU >= 3
            case CEU_VALUE_CLO_CORO:
                printf("coro: %p", v.Dyn);
                break;
    #endif
    #if CEU >= 4
            case CEU_VALUE_CLO_TASK:
                printf("task: %p", v.Dyn);
                break;
    #endif
    #if CEU >= 3
            case CEU_VALUE_EXE_CORO:
                printf("exe-coro: %p", v.Dyn);
                break;
    #endif
    #if CEU >= 4
            case CEU_VALUE_EXE_TASK:
                printf("exe-task: %p", v.Dyn);
                break;
    #endif
    #if CEU >= 5
            case CEU_VALUE_TASKS:
                printf("tasks: %p", v.Dyn);
                break;
            case CEU_VALUE_TRACK:
                printf("track: %p", v.Dyn);
                break;
    #endif
            default:
                assert(0 && "bug found");
        }
    }
    int ceu_print_f (CEUX* X) {
        for (int i=0; i<X->args; i++) {
            if (i > 0) {
                printf("\t");
            }
            ceu_print1(ceux_peek(X->S, ceux_arg(X,i)));
        }
        return 0;
    }
    int ceu_println_f (CEUX* X) {
        assert(0 == ceu_print_f(X));
        printf("\n");
        return 0;
    }
    
    CEU_Value _ceu_equals_equals_ (CEU_Value e1, CEU_Value e2) {
        int v = (e1.type == e2.type);
        if (v) {
            switch (e1.type) {
                case CEU_VALUE_NIL:
                    v = 1;
                    break;
                case CEU_VALUE_TAG:
                    v = (e1.Tag == e2.Tag);
                    break;
                case CEU_VALUE_BOOL:
                    v = (e1.Bool == e2.Bool);
                    break;
                case CEU_VALUE_CHAR:
                    v = (e1.Char == e2.Char);
                    break;
                case CEU_VALUE_NUMBER:
                    v = (e1.Number == e2.Number);
                    break;
                case CEU_VALUE_POINTER:
                    v = (e1.Pointer == e2.Pointer);
                    break;
                case CEU_VALUE_TUPLE:
                case CEU_VALUE_VECTOR:
                case CEU_VALUE_DICT:
                case CEU_VALUE_CLO_FUNC:
        #if CEU >= 3
                case CEU_VALUE_CLO_CORO:
        #endif
        #if CEU >= 4
                case CEU_VALUE_CLO_TASK:
        #endif
        #if CEU >= 3
                case CEU_VALUE_EXE_CORO:
        #endif
        #if CEU >= 4
                case CEU_VALUE_EXE_TASK:
        #endif
        #if CEU >= 5
                case CEU_VALUE_TRACK:
        #endif
                    v = (e1.Dyn == e2.Dyn);
                    break;
                default:
                    assert(0 && "bug found");
            }
        }
        return (CEU_Value) { CEU_VALUE_BOOL, {.Bool=v} };
    }
    int ceu_equals_equals_f (CEUX* X) {
        assert(X->args == 2);
        CEU_Value ret = _ceu_equals_equals_(ceux_peek(X->S, ceux_arg(X,0)), ceux_peek(X->S, ceux_arg(X,1)));
        ceux_push(X->S, 1, ret);
        return 1;
    }
    int ceu_slash_equals_f (CEUX* X) {
        ceu_equals_equals_f(X);
        CEU_Value ret = ceux_pop(X->S, 0);
        assert(ret.type == CEU_VALUE_BOOL);
        ret.Bool = !ret.Bool;
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
    int ceu_hash_f (CEUX* X) {
        assert(X->args == 1);
        CEU_Value v = ceux_peek(X->S, ceux_arg(X,0));
        CEU_Value ret;
        if (v.type == CEU_VALUE_VECTOR) {
            ret = (CEU_Value) { CEU_VALUE_NUMBER, {.Number=v.Dyn->Vector.its} };
        } else if (v.type == CEU_VALUE_TUPLE) {
            ret = (CEU_Value) { CEU_VALUE_NUMBER, {.Number=v.Dyn->Tuple.its} };
        } else {
            return ceu_error_s(X->S, "length error : not a vector");
        }
        ceux_push(X->S, 1, ret);
        return 1;
    }
    
        #if CEU >= 3
        int ceu_isexe_val (CEU_Value val) {
            return (val.type==CEU_VALUE_EXE_CORO CEU4(|| ceu_istask_val(val)));
        }
        int ceu_isexe_dyn (CEU_Dyn* dyn) {
            return (dyn->Any.type==CEU_VALUE_EXE_CORO CEU4(|| ceu_istask_dyn(dyn)));
        }
        int ceu_coroutine_f (CEUX* X) {
            assert(X->args == 1);
            CEU_Value coro = ceux_peek(X->S, ceux_arg(X,0));
            CEU_Value ret;
            if (coro.type != CEU_VALUE_CLO_CORO) {
                return ceu_error_s(X->S, "coroutine error : expected coro");
            } else {
                ret = ceu_create_exe(CEU_VALUE_EXE_CORO, sizeof(CEU_Exe), coro);
            }
            ceux_push(X->S, 1, ret);
            return 1;
        }        

        int ceu_status_f (CEUX* X) {
            assert(X->args == 1);
            CEU_Value exe = ceux_peek(X->S, ceux_arg(X,0));
            CEU_Value ret;
            if (exe.type!=CEU_VALUE_EXE_CORO CEU4(&& !ceu_istask_val(exe))) {
        #if CEU < 4
                return ceu_error_s(X->S, "status error : expected running coroutine");
        #else
                return ceu_error_s(X->S, "status error : expected running coroutine or task");
        #endif
            } else {
                ret = (CEU_Value) { CEU_VALUE_TAG, {.Tag=exe.Dyn->Exe.status + CEU_TAG_yielded - 1} };
            }
            ceux_push(X->S, 1, ret);
            return 1;
        }
        
        int ceu_exe_term (CEUX* X) {
            if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                // leave -> outer ref -> gc_dec -> term
                return 0;
            }
            X->exe->status = CEU_EXE_STATUS_TERMINATED;
            int ret = 0;
    #if CEU >= 4
            if (X->exe->type == CEU_VALUE_EXE_TASK) {
                // do not bcast aborted task b/c
                // it would awake parents that actually need to
                // respond/catch the error (thus not awake)
                if (X->action != CEU_ACTION_ABORT) {
                    CEU_Exe_Task* tsk = ((CEU_Exe_Task*) X->exe);
                    CEU_Dyn* up;
    #if CEU >= 5
                    if (tsk->lnks.up.dyn!=NULL && tsk->lnks.up.dyn->Any.type==CEU_VALUE_TASKS) {
                        // tsk <- pool <- tsk
                        up = CEU_LNKS(tsk->lnks.up.dyn)->up.dyn;
                    } else
    #endif
                    {
                        // tsk <- tsk
                        up = tsk->lnks.up.dyn;
                    }
                    if (up!=NULL && !CEU_ERROR_IS(X->S)) {
                        assert(CEU_TIME < UINT32_MAX);
                        CEU_TIME++;
                        int i = ceux_push(X->S, 1, ceu_dyn_to_val((CEU_Dyn*) X->exe));   // bcast myself
                        ret = ceu_bcast_dyn(X, CEU_ACTION_RESUME, CEU_TIME, up);
                        //assert(ret == 0);
                        assert(X->exe->refs >= 2);  // ensures that the unlink below is safe (otherwise call gc_inc)
                        ceux_rem(X->S, i);
                    }
                    ceu_gc_dec_dyn((CEU_Dyn*) X->exe);  // only if natural termination
                }
            }
    #endif
            return ret;
        }

        #if CEU >= 5
        void ceu_abort_tasks (CEU_Tasks* tsks) {
            if (tsks->lnks.up.dyn == NULL) {
                return;     // already unlinked/killed
            }
            CEU_Dyn* cur = tsks->lnks.dn.fst;
            while (cur != NULL) {
                int term = (cur->Exe_Task.status == CEU_EXE_STATUS_TERMINATED);
                if (!term) {
                    ceu_abort_exe((CEU_Exe*) cur);
                }
                CEU_Dyn* nxt = CEU_LNKS(cur)->sd.nxt;
                if (!term) {
                    ceu_gc_dec_dyn(cur); // remove strong ref // TODO: could affect nxt?
                }
                cur = nxt;
            }
        }
        #endif

        void ceu_abort_exe (CEU_Exe* exe) {
            assert(ceu_isexe_dyn((CEU_Dyn*) exe));
            switch (exe->status) {
                case CEU_EXE_STATUS_TERMINATED:
                    // do nothing;
                    break;
                case CEU_EXE_STATUS_RESUMED:
                    exe->status = CEU_EXE_STATUS_TERMINATED;
                    break;
        #if CEU >= 4
                case CEU_EXE_STATUS_TOGGLED:
        #endif
                case CEU_EXE_STATUS_YIELDED:
                {
                    // TODO - fake S/X - should propagate up to calling stack
                    // TODO - fake now - should receive as arg (not CEU_TIME)
                    CEU_Stack S = { 0, {} };
                    CEUX _X = { &S, -1, -1, CEU_ACTION_INVALID, {.exe=NULL} CEU4(COMMA CEU_TIME COMMA NULL) };
                    CEUX* X = &_X;
                    ceux_push(&S, 1, ceu_dyn_to_val((CEU_Dyn*) exe));
                    // S: [co]
                    int ret = ceux_resume(X, 0, 0, CEU_ACTION_ABORT CEU4(COMMA CEU_TIME));
                    if (ret != 0) {
                        assert(CEU_ERROR_IS(&S) && "TODO: abort should not return");
                        assert(0 && "TODO: error in ceu_exe_kill");
                    }
                }
            }
        }
        #endif
     // TASK
        #if CEU >= 4
        void ceu_dyn_unlink (CEU_Dyn* dyn) {
            CEU_Links* me_lnks = CEU_LNKS(dyn);
            {   // UP-DYN-DN
                if (me_lnks->up.dyn != NULL) {
                    CEU_Links* up_lnks = CEU_LNKS(me_lnks->up.dyn);
                    me_lnks->up.dyn = NULL;
                    if (up_lnks->dn.fst == dyn) {
                        assert(me_lnks->sd.prv == NULL);
                        up_lnks->dn.fst = me_lnks->sd.nxt;
                    }
                    if (up_lnks->dn.lst == dyn) {
                        assert(me_lnks->sd.nxt == NULL);
                        up_lnks->dn.lst = me_lnks->sd.prv;
                    }
                }
            }
            {   // UP-BLK-DN
                if (me_lnks->up.blk != NULL) {
                    *me_lnks->up.blk = me_lnks->sd.nxt;
                    if (me_lnks->sd.nxt != NULL) {
                        CEU_LNKS(me_lnks->sd.nxt)->up.blk = me_lnks->up.blk;
                    }
                    me_lnks->up.blk = NULL; // also on ceux_block_leave (to prevent dangling pointer)
                }
            }
            {   // SD
                if (me_lnks->sd.prv != NULL) {
                    CEU_LNKS(me_lnks->sd.prv)->sd.nxt = me_lnks->sd.nxt;
                }
                if (me_lnks->sd.nxt != NULL) {
                    CEU_LNKS(me_lnks->sd.nxt)->sd.prv = me_lnks->sd.prv;
                }
                //me_lnks->sd.prv = me_lnks->sd.nxt = NULL;
                    // prv/nxt are never reached again:
                    //  - it is not a problem to keep the dangling pointers
                    // but we actually should not set them NULL:
                    //  - tsk might be in bcast_tasks which must call nxt
            }
            {   // DN
                CEU_Dyn* cur = me_lnks->dn.fst;
                if (me_lnks->dn.fst == NULL) {
                    assert(me_lnks->dn.lst == NULL);
                }
                while (cur != NULL) {
                    CEU_Links* dn_lnks = CEU_LNKS(cur);
                    dn_lnks->up.dyn = NULL;
                    cur = dn_lnks->sd.nxt;
                }
                me_lnks->dn.fst = me_lnks->dn.lst = NULL;
            }
        }
        
        int ceu_istask_dyn (CEU_Dyn* dyn) {
            return (dyn->Any.type == CEU_VALUE_EXE_TASK);
        }
        int ceu_istask_val (CEU_Value val) {
            return (val.type>CEU_VALUE_DYNAMIC) && ceu_istask_dyn(val.Dyn);
        }
        #endif
    
        #if CEU >= 4
        int ceu_bcast_tasks (CEUX* X1, CEU_ACTION act, uint32_t now, CEU_Dyn* dyn2) {
            //assert(dyn2!=NULL && (dyn2->type==CEU_VALUE_EXE_TASK CEU5(|| dyn2->type==CEU_VALUE_TASKS)));
            int ret = 0;
            CEU_Links* lnks = CEU_LNKS(dyn2);
            CEU_Dyn* cur = lnks->dn.fst;
            while (cur != NULL) {
                ceu_gc_inc_dyn(cur);
                ret = ceu_bcast_dyn(X1, act, now, cur);
                CEU_Dyn* nxt = CEU_LNKS(cur)->sd.nxt;
                ceu_gc_dec_dyn(cur); // TODO: could affect nxt?
                if (ret != 0) {
                    break;
                }
                cur = nxt;
            }
            return ret;
        }
        int ceu_bcast_task (CEUX* X1, CEU_ACTION act, uint32_t now, CEU_Exe_Task* tsk2) {            
            // bcast order: DN -> ME
            //  - DN:  nested tasks
            //  - ME:  my yield point
            
            // X1: [evt]    // must keep as is at the end bc outer bcast pops it
            
            assert(tsk2!=NULL && tsk2->type==CEU_VALUE_EXE_TASK);
            assert(act == CEU_ACTION_RESUME);
            
            if (tsk2->status == CEU_EXE_STATUS_TERMINATED) {
                return 0;
            }
            
            ceu_gc_inc_dyn((CEU_Dyn*) tsk2);
            int ret = 0; // !=0 means error

            // DN
            if (act==CEU_ACTION_RESUME && tsk2->status==CEU_EXE_STATUS_TOGGLED) {
                // do nothing
            } else {
                ret = ceu_bcast_tasks(X1, act, now, (CEU_Dyn*) tsk2);
            }

            // ME
            if (tsk2->status != CEU_EXE_STATUS_YIELDED) {
                // do nothing
            } else if (tsk2 == &CEU_GLOBAL_TASK) {
                // do nothing
            } else {
                // either handle error or event
                // never both
                // even if error is caught, should not awake from past event
                if (ret != 0) {
                    // catch error from blocks above
                    assert(CEU_ERROR_IS(X1->S));
                    // [evt, (ret,err)]
                    ceux_push(X1->S, 1, ceu_dyn_to_val((CEU_Dyn*)tsk2));
                    int err = XX1(-ret-1);
                    ceux_dup_n(X1->S, err, ret);
                    // [evt, (ret,err), tsk, (ret,err)]
                    int ret2 = ceux_resume(X1, ret, 0, CEU_ACTION_ERROR, now);
                    if (ret2 == 0) {
                        ceux_pop_n(X1->S, ret);
                        // [evt]
                    } else {
                        // [evt, (ret,err)]
                    }
                    ret = ret2;
                } else if (tsk2->pc==0 || now>tsk2->time) {
                    // [evt]
                    ceux_push(X1->S, 1, ceu_dyn_to_val((CEU_Dyn*)tsk2));
                    ceux_dup(X1->S, XX1(-2));
                    // [evt,tsk,evt]
                    ret = ceux_resume(X1, 1 /* TODO-MULTI */, 0, CEU_ACTION_RESUME, now);
                    // [evt]
                }
            }
            
            ceu_gc_dec_dyn((CEU_Dyn*) tsk2);
            return ret;
        }

        int ceu_broadcast_global (void) {
            assert(CEU_TIME < UINT32_MAX);
            CEU_TIME++;
            int ret = ceu_bcast_tasks(CEU_GLOBAL_X, CEU_ACTION_RESUME, CEU_TIME, (CEU_Dyn*) &CEU_GLOBAL_TASK);
            return ret;
        }
        
        int ceu_broadcast_plic__f (CEUX* X) {
            assert(X->args == 2);
            //ceu_bstk_assert(bstk);

            assert(CEU_TIME < UINT32_MAX);
            CEU_TIME++;

            CEU_Value xin = ceux_peek(X->S, ceux_arg(X,0));
            int ret;
            if (xin.type == CEU_VALUE_TAG) {
                if (xin.Tag == CEU_TAG_global) {
                    ret = ceu_bcast_tasks(X, CEU_ACTION_RESUME, CEU_TIME, (CEU_Dyn*) &CEU_GLOBAL_TASK);
                } else if (xin.Tag == CEU_TAG_task) {
                    if (X->exe_task == NULL) {
                        ret = ceu_bcast_tasks(X, CEU_ACTION_RESUME, CEU_TIME, (CEU_Dyn*) &CEU_GLOBAL_TASK);
                    } else {
                        ret = ceu_bcast_task(X, CEU_ACTION_RESUME, CEU_TIME, X->exe_task);
                    }
                } else {
                    ret = ceu_error_s(X->S, "broadcast error : invalid target");
                }
            } else {
                if (ceu_istask_val(xin)) {
                    if (xin.Dyn->Exe_Task.status == CEU_EXE_STATUS_TERMINATED) {
                        ret = 0;
                    } else {
                        ret = ceu_bcast_task(X, CEU_ACTION_RESUME, CEU_TIME, &xin.Dyn->Exe_Task);
                    }
                } else {
                    ret = ceu_error_s(X->S, "broadcast error : invalid target");
                }
            }

            return ret;
        }
        #endif
    
        #if CEU >= 5
        int ceu_next_dash_tasks_f (CEUX* X) {
            assert(X->args==1 || X->args==2);
            CEU_Value tsks = ceux_peek(X->S, ceux_arg(X,0));
            if (tsks.type != CEU_VALUE_TASKS) {
                return ceu_error_s(X->S, "next-tasks error : expected tasks");
            }
            CEU_Value key = (X->args == 1) ? ((CEU_Value) { CEU_VALUE_NIL }) : ceux_peek(X->S,ceux_arg(X,1));
            CEU_Dyn* nxt = NULL;
            switch (key.type) {
                case CEU_VALUE_NIL:
                    nxt = tsks.Dyn->Tasks.lnks.dn.fst;
                    break;
                case CEU_VALUE_EXE_TASK:
                    nxt = key.Dyn->Exe_Task.lnks.sd.nxt;
                    break;
                default:
                    return ceu_error_s(X->S, "next-tasks error : expected task");
            }
            if (nxt == NULL) {
                ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            } else {
                ceux_push(X->S, 1, ceu_dyn_to_val(nxt));
            }
            return 1;
        }
        int ceu_tasks_f (CEUX* X) {
            assert(X->args <= 1);
            int max = 0;
            if (X->args == 1) {
                CEU_Value xmax = ceux_peek(X->S, ceux_arg(X,0));
                if (xmax.type!=CEU_VALUE_NUMBER || xmax.Number<=0) {                
                    return ceu_error_s(X->S, "tasks error : expected positive number");
                }
                max = xmax.Number;
            }
            CEU_Value ret = ceu_create_tasks(max, ceu_up_tsk(X), ceu_up_blk(X->S));
            ceux_push(X->S, 1, ret);
            return 1;
        }
        #endif
    
    
                    // PROTO | (prelude.ceu : lin 3 : col 1) | (func (v) pri
                    int ceu_f_debug (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 3 : col 16) | println(v)v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 4 : col 5) | println(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 4) /* global println */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 4, col 5) : println(v)");
            
        
                } // CALL | (prelude.ceu : lin 4 : col 5) | println(v)
                ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 3 : col 16) | println(v)v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 12 : col 5) | (func (v1,v2) 
                    int ceu_f_plus (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 12 : col 25) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 13 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 14 : col 21) | do (val ceu_7
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 15 : col 25) | (val ceu_77 = 
                
                    
                { // CALL | (prelude.ceu : lin 13 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 13 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 13, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 13 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 13, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 13 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_77 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 15 : col 25) | (val ceu_77 = )
        
            
                { // IF | (prelude.ceu : lin 14 : col 25) | if ceu_77 ceu
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_77 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 14 : col 35) | ceu_77
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_77 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 14 : col 35) | ceu_77
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 16 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 13 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 13 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 13, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 13 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 13, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 13 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 16 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 14 : col 21) | do (val ceu_7
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 13 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 14 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 14, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 14 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 13 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 13 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 13 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 13 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number + (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 12 : col 25) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 19 : col 5) | (func (v1,v2) 
                    int ceu_f_minus (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 19 : col 25) | do if do 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 25 : col 21) | do if do (v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 29 : col 29) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 22 : col 21) | do (val ceu_2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 23 : col 25) | (val ceu_285 = 
                
                    
                { // CALL | (prelude.ceu : lin 21 : col 23) | ==(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 21 : col 14) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 21, col 14) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 21 : col 14) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 21, col 23) : {{==}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 21 : col 23) | ==(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_285 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 23 : col 25) | (val ceu_285 = )
        
            
                { // IF | (prelude.ceu : lin 22 : col 25) | if ceu_285 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_285 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 22 : col 36) | ==(v2,nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 21 : col 43) | ==(v2,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 21, col 43) : {{==}}(v2,nil)");
            
        
                } // CALL | (prelude.ceu : lin 21 : col 43) | ==(v2,nil)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 22 : col 36) | ==(v2,nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 22 : col 32) | ceu_285
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_285 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 22 : col 32) | ceu_285
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 22 : col 21) | do (val ceu_2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 25 : col 3) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(   (- (ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 25 : col 3) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 23 : col 36) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 26 : col 29) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 23 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 24 : col 25) | (val ceu_365 = 
                
                    
                { // CALL | (prelude.ceu : lin 22 : col 23) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 22 : col 14) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 22, col 14) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 22 : col 14) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 22, col 23) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 22 : col 23) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_365 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 24 : col 25) | (val ceu_365 = )
        
            
                { // IF | (prelude.ceu : lin 23 : col 25) | if ceu_365 ce
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_365 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 23 : col 36) | ceu_365
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_365 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 23 : col 36) | ceu_365
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 25 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 22 : col 49) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 22 : col 40) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 22, col 40) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 22 : col 40) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 22, col 49) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 22 : col 49) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 25 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 23 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 24 : col 3) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 22 : col 64) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 22, col 64) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 22 : col 64) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 24 : col 3) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 24 : col 36) | if true ```
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 27 : col 29) | if true ```:n
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 23 : col 18) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(   ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number - (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 23 : col 18) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 25 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 25 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 25 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 24 : col 36) | if true ```
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 23 : col 36) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 25 : col 21) | do if do (v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 19 : col 25) | do if do 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 27 : col 5) | (func (v1,v2) 
                    int ceu_f_asterisk (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 27 : col 25) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 28 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 29 : col 21) | do (val ceu_8
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 30 : col 25) | (val ceu_807 = 
                
                    
                { // CALL | (prelude.ceu : lin 28 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 28 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 28, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 28 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 28, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 28 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_807 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 30 : col 25) | (val ceu_807 = )
        
            
                { // IF | (prelude.ceu : lin 29 : col 25) | if ceu_807 ce
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_807 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 29 : col 36) | ceu_807
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_807 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 29 : col 36) | ceu_807
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 31 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 28 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 28 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 28, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 28 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 28, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 28 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 31 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 29 : col 21) | do (val ceu_8
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 28 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 29 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 29, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 29 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 28 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 28 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 28 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 28 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number * (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 27 : col 25) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 34 : col 5) | (func (v1,v2) 
                    int ceu_f_asterisk_asterisk (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 34 : col 26) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 35 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 36 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 37 : col 25) | (val ceu_1014 =
                
                    
                { // CALL | (prelude.ceu : lin 35 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 35 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 35, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 35 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 35, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 35 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1014 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 37 : col 25) | (val ceu_1014 =)
        
            
                { // IF | (prelude.ceu : lin 36 : col 25) | if ceu_1014 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1014 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 36 : col 37) | ceu_1014
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1014 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 36 : col 37) | ceu_1014
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 38 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 35 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 35 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 35, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 35 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 35, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 35 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 38 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 36 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 35 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 36 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 36, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 36 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 35 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 35 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 35 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 35 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  powf((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number, (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 34 : col 26) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 41 : col 5) | (func (v1,v2) 
                    int ceu_f_slash (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 41 : col 25) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 42 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 43 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 44 : col 25) | (val ceu_1221 =
                
                    
                { // CALL | (prelude.ceu : lin 42 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 42 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 42, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 42 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 42, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 42 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1221 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 44 : col 25) | (val ceu_1221 =)
        
            
                { // IF | (prelude.ceu : lin 43 : col 25) | if ceu_1221 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1221 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 43 : col 37) | ceu_1221
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1221 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 43 : col 37) | ceu_1221
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 45 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 42 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 42 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 42, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 42 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 42, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 42 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 45 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 43 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 42 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 43 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 43, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 43 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 42 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 42 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 42 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 42 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number / (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 41 : col 25) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 48 : col 5) | (func (v1,v2) 
                    int ceu_f_slash_slash (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 48 : col 26) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 49 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 50 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 51 : col 25) | (val ceu_1428 =
                
                    
                { // CALL | (prelude.ceu : lin 49 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 49 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 49, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 49 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 49, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 49 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1428 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 51 : col 25) | (val ceu_1428 =)
        
            
                { // IF | (prelude.ceu : lin 50 : col 25) | if ceu_1428 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1428 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 50 : col 37) | ceu_1428
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1428 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 50 : col 37) | ceu_1428
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 52 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 49 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 49 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 49, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 49 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 49, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 49 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 52 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 50 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 49 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 50 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 50, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 50 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 49 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 49 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 49 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 49 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  powf((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number, 1/(ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 48 : col 26) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 55 : col 5) | (func (v1,v2) 
                    int ceu_f_null (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 55 : col 25) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 56 : col 9) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 57 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 58 : col 25) | (val ceu_1635 =
                
                    
                { // CALL | (prelude.ceu : lin 56 : col 22) | /=(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 56 : col 13) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 56, col 13) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 56 : col 13) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 56, col 22) : {{/=}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 56 : col 22) | /=(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1635 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 58 : col 25) | (val ceu_1635 =)
        
            
                { // IF | (prelude.ceu : lin 57 : col 25) | if ceu_1635 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1635 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 57 : col 37) | ceu_1635
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1635 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 57 : col 37) | ceu_1635
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 59 : col 32) | /=(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 56 : col 47) | /=(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 56 : col 38) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 56, col 38) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 56 : col 38) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 56, col 47) : {{/=}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 56 : col 47) | /=(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 59 : col 32) | /=(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 57 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 56 : col 59) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 57 : col 13) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 57, col 13) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 57 : col 13) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 56 : col 59) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 56 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 56 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 56 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  fmod((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number, (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 55 : col 25) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 66 : col 5) | (func (v1,v2) 
                    int ceu_f_greater (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 66 : col 24) | do if do 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 72 : col 21) | do if do (v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 76 : col 29) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 69 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 70 : col 25) | (val ceu_1843 =
                
                    
                { // CALL | (prelude.ceu : lin 68 : col 23) | ==(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 68 : col 14) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 68, col 14) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 68 : col 14) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 68, col 23) : {{==}}(type(v1),:tag)");
            
        
                } // CALL | (prelude.ceu : lin 68 : col 23) | ==(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1843 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 70 : col 25) | (val ceu_1843 =)
        
            
                { // IF | (prelude.ceu : lin 69 : col 25) | if ceu_1843 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1843 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 69 : col 37) | ==(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 68 : col 49) | ==(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 68 : col 40) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 68, col 40) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 68 : col 40) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 68, col 49) : {{==}}(type(v2),:tag)");
            
        
                } // CALL | (prelude.ceu : lin 68 : col 49) | ==(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 69 : col 37) | ==(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 69 : col 32) | ceu_1843
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1843 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 69 : col 32) | ceu_1843
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 69 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 72 : col 3) | ```:bool    (
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_BOOL, {.Bool=(   ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Tag    > (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Tag))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 72 : col 3) | ```:bool    (
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 70 : col 36) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 73 : col 29) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 70 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 71 : col 25) | (val ceu_1933 =
                
                    
                { // CALL | (prelude.ceu : lin 69 : col 23) | ==(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 69 : col 14) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 69, col 14) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 69 : col 14) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 69, col 23) : {{==}}(type(v1),:number)");
            
        
                } // CALL | (prelude.ceu : lin 69 : col 23) | ==(type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_1933 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 71 : col 25) | (val ceu_1933 =)
        
            
                { // IF | (prelude.ceu : lin 70 : col 25) | if ceu_1933 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1933 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 70 : col 37) | ==(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 69 : col 49) | ==(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 69 : col 40) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 69, col 40) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 69 : col 40) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 69, col 49) : {{==}}(type(v2),:number)");
            
        
                } // CALL | (prelude.ceu : lin 69 : col 49) | ==(type(v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 70 : col 37) | ==(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 70 : col 32) | ceu_1933
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_1933 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 70 : col 32) | ceu_1933
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 70 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 73 : col 3) | ```:bool    (
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_BOOL, {.Bool=(   ((ceux_peek(X->S,ceux_arg(X, 0) /* arg v1 */)).Number > (ceux_peek(X->S,ceux_arg(X, 1) /* arg v2 */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 73 : col 3) | ```:bool    (
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 71 : col 36) | if true err
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 74 : col 29) | if true error
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 70 : col 18) | error(:error)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 70 : col 21) | error(:error)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_error} });
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 70, col 21) : error(:error)");
            
        
                } // CALL | (prelude.ceu : lin 70 : col 21) | error(:error)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 70 : col 18) | error(:error)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 72 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 72 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 72 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 71 : col 36) | if true err
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 70 : col 36) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 72 : col 21) | do if do (v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 66 : col 24) | do if do 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 74 : col 5) | (func (v1,v2) 
                    int ceu_f_less (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 74 : col 24) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 75 : col 10) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 76 : col 21) | do (val ceu_2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 77 : col 25) | (val ceu_2381 =
                
                    
                { // CALL | (prelude.ceu : lin 75 : col 18) | ==(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 75, col 18) : {{==}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 75 : col 18) | ==(v1,v2)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_2381 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 77 : col 25) | (val ceu_2381 =)
        
            
                { // IF | (prelude.ceu : lin 76 : col 25) | if ceu_2381 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2381 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 76 : col 37) | ceu_2381
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2381 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 76 : col 37) | ceu_2381
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 78 : col 32) | >(v1,v2)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 75 : col 32) | >(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 75, col 32) : {{>}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 75 : col 32) | >(v1,v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 78 : col 32) | >(v1,v2)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 76 : col 21) | do (val ceu_2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 77 : col 3) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 77 : col 3) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 77 : col 18) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 77 : col 18) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 74 : col 24) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 78 : col 5) | (func (v1,v2) 
                    int ceu_f_greater_equals (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 78 : col 25) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 80 : col 21) | do (val ceu_2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 81 : col 25) | (val ceu_2597 =
                
                    
                { // CALL | (prelude.ceu : lin 79 : col 13) | ==(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 79, col 13) : {{==}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 79 : col 13) | ==(v1,v2)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_2597 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 81 : col 25) | (val ceu_2597 =)
        
            
                { // IF | (prelude.ceu : lin 80 : col 25) | if ceu_2597 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2597 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 80 : col 37) | ceu_2597
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2597 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 80 : col 37) | ceu_2597
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 82 : col 32) | >(v1,v2)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 79 : col 27) | >(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 79, col 27) : {{>}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 79 : col 27) | >(v1,v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 82 : col 32) | >(v1,v2)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 80 : col 21) | do (val ceu_2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 78 : col 25) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 82 : col 5) | (func (v1,v2) 
                    int ceu_f_less_equals (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 82 : col 25) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 84 : col 21) | do (val ceu_2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 85 : col 25) | (val ceu_2737 =
                
                    
                { // CALL | (prelude.ceu : lin 83 : col 13) | ==(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 83, col 13) : {{==}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 83 : col 13) | ==(v1,v2)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_2737 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 85 : col 25) | (val ceu_2737 =)
        
            
                { // IF | (prelude.ceu : lin 84 : col 25) | if ceu_2737 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2737 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 84 : col 37) | ceu_2737
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_2737 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 84 : col 37) | ceu_2737
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 86 : col 32) | <(v1,v2)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 83 : col 27) | <(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 30) /* global {{<}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 83, col 27) : {{<}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 83 : col 27) | <(v1,v2)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 86 : col 32) | <(v1,v2)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 84 : col 21) | do (val ceu_2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 82 : col 25) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 89 : col 1) | (func (v,msg) 
                    int ceu_f_assert (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 89 : col 22) | if if v fal
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 90 : col 5) | if if v false
                    
                { // IF | (prelude.ceu : lin 90 : col 9) | if v false 
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 90 : col 14) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 90 : col 14) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 90 : col 29) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 90 : col 29) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 90 : col 14) | if msg prin
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 91 : col 9) | if msg print(
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg msg */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 91 : col 16) | print(#['a','
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 92 : col 13) | print(#['a','s'
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 3) /* global print */));

                    
                        
                { // VECTOR | (prelude.ceu : lin 92 : col 19) | #['a','s','s','
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='a'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='s'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='s'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='e'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='r'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 4, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='t'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 5, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='i'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 6, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='o'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 7, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='n'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 8, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char=' '} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 9, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='e'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 10, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='r'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 11, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='r'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 12, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='o'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 13, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='r'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 14, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char=' '} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 15, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char=':'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 16, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char=' '} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 17, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 92, col 13) : print(#['a','s','s','e','r','t','i','o','n','...)");
            
        
                } // CALL | (prelude.ceu : lin 92 : col 13) | print(#['a','s'
                
                { // CALL | (prelude.ceu : lin 93 : col 13) | println(msg)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 4) /* global println */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg msg */));

                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 93, col 13) : println(msg)");
            
        
                } // CALL | (prelude.ceu : lin 93 : col 13) | println(msg)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 91 : col 16) | print(#['a','
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 91 : col 9) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 91 : col 9) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 91 : col 9) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                { // CALL | (prelude.ceu : lin 95 : col 9) | error(:assert)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_assert} });
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 95, col 9) : error(:assert)");
            
        
                } // CALL | (prelude.ceu : lin 95 : col 9) | error(:assert)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 90 : col 14) | if msg prin
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 90 : col 5) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 90 : col 5) | nil)
        
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 90 : col 5) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 89 : col 22) | if if v fal
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 102 : col 1) | (func (t1,t2) 
                    int ceu_f_tag_dash_or (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 102 : col 21) | assert(==
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 103 : col 5) | assert(==(t
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 33) /* global assert */));

                    
                        
                { // CALL | (prelude.ceu : lin 103 : col 21) | ==(type(t1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 103 : col 12) | type(t1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg t1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 103, col 12) : type(t1)");
            
        
                } // CALL | (prelude.ceu : lin 103 : col 12) | type(t1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 103, col 21) : {{==}}(type(t1),:tag)");
            
        
                } // CALL | (prelude.ceu : lin 103 : col 21) | ==(type(t1)
                
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 103, col 5) : assert({{==}}(type(t1),:tag))");
            
        
                } // CALL | (prelude.ceu : lin 103 : col 5) | assert(==(t
                
                { // CALL | (prelude.ceu : lin 104 : col 5) | assert(==(t
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 33) /* global assert */));

                    
                        
                { // CALL | (prelude.ceu : lin 104 : col 21) | ==(type(t2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 104 : col 12) | type(t2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg t2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 104, col 12) : type(t2)");
            
        
                } // CALL | (prelude.ceu : lin 104 : col 12) | type(t2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 104, col 21) : {{==}}(type(t2),:tag)");
            
        
                } // CALL | (prelude.ceu : lin 104 : col 21) | ==(type(t2)
                
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 104, col 5) : assert({{==}}(type(t2),:tag))");
            
        
                } // CALL | (prelude.ceu : lin 104 : col 5) | assert(==(t
                ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_TAG, {.Tag=(  (ceux_peek(X->S,ceux_arg(X, 0) /* arg t1 */)).Tag | (ceux_peek(X->S,ceux_arg(X, 1) /* arg t2 */)).Tag)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 102 : col 21) | assert(==
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 112 : col 1) | (func (v1,v2) 
                    int ceu_f_min (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 112 : col 19) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 114 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 115 : col 25) | (val ceu_3293 =
                
                    
                { // BLOCK | (prelude.ceu : lin 114 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 115 : col 25) | (val ceu_3252 =
                
                    
                { // CALL | (prelude.ceu : lin 113 : col 10) | <(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 30) /* global {{<}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 113, col 10) : {{<}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 113 : col 10) | <(v1,v2)
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_3252 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 115 : col 25) | (val ceu_3252 =)
        
            
                { // IF | (prelude.ceu : lin 114 : col 25) | if ceu_3252 v
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_3252 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 114 : col 37) | v1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 114 : col 37) | v1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 114 : col 32) | ceu_3252
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_3252 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 114 : col 32) | ceu_3252
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 114 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_3293 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 115 : col 25) | (val ceu_3293 =)
        
            
                { // IF | (prelude.ceu : lin 118 : col 25) | if ceu_3293 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_3293 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 118 : col 37) | ceu_3293
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_3293 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 118 : col 37) | ceu_3293
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 120 : col 32) | v2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 120 : col 32) | v2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 114 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 112 : col 19) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 116 : col 1) | (func (v1,v2) 
                    int ceu_f_max (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 116 : col 19) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 118 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 119 : col 25) | (val ceu_3503 =
                
                    
                { // BLOCK | (prelude.ceu : lin 118 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 119 : col 25) | (val ceu_3462 =
                
                    
                { // CALL | (prelude.ceu : lin 117 : col 10) | >(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 117, col 10) : {{>}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 117 : col 10) | >(v1,v2)
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_3462 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 119 : col 25) | (val ceu_3462 =)
        
            
                { // IF | (prelude.ceu : lin 118 : col 25) | if ceu_3462 v
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_3462 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 118 : col 37) | v1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 118 : col 37) | v1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 118 : col 32) | ceu_3462
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_3462 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 118 : col 32) | ceu_3462
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 118 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_3503 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 119 : col 25) | (val ceu_3503 =)
        
            
                { // IF | (prelude.ceu : lin 122 : col 25) | if ceu_3503 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_3503 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 122 : col 37) | ceu_3503
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_3503 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 122 : col 37) | ceu_3503
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 124 : col 32) | v2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 124 : col 32) | v2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 118 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 116 : col 19) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 120 : col 1) | (func (min_max,
                    int ceu_f_between (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 120 : col 27) | max(min_max[0
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 121 : col 5) | max(min_max[0],
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 36) /* global max */));

                    
                        
                { // INDEX | (prelude.ceu : lin 121 : col 9) | min_max[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg min_max */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 121, col 9) : min_max[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 121, col 9) : min_max[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        
                { // CALL | (prelude.ceu : lin 121 : col 21) | min(min_max[1],
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 35) /* global min */));

                    
                        
                { // INDEX | (prelude.ceu : lin 121 : col 25) | min_max[1]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg min_max */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 121, col 25) : min_max[1]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 121, col 25) : min_max[1]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 121, col 21) : min(min_max[1],v)");
            
        
                } // CALL | (prelude.ceu : lin 121 : col 21) | min(min_max[1],
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 121, col 5) : max(min_max[0],min(min_max[1],v))");
            
        
                } // CALL | (prelude.ceu : lin 121 : col 5) | max(min_max[0],
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 120 : col 27) | max(min_max[0
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 131 : col 5) | (func (itr :Ite
                    int ceu_f_iter_dash_tuple (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 131 : col 37) | (val i = itr[
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 132 : col 9) | (val i = itr[:i
                
                    
                { // INDEX | (prelude.ceu : lin 132 : col 17) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 132, col 17) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 132, col 17) : itr[:i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local i */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 132 : col 9) | (val i = itr[:i)
        
            
                { // IF | (prelude.ceu : lin 133 : col 9) | if >=(i,#
                    
                { // CALL | (prelude.ceu : lin 133 : col 14) | >=(i,#(
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 31) /* global {{>=}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local i */));

                    
                        
                { // CALL | (prelude.ceu : lin 133 : col 17) | #(itr[:s])
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        
                { // INDEX | (prelude.ceu : lin 133 : col 18) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 133, col 18) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 133, col 18) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 133, col 17) : {{#}}(itr[:s])");
            
        
                } // CALL | (prelude.ceu : lin 133 : col 17) | #(itr[:s])
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 133, col 14) : {{>=}}(i,{{#}}(itr[:s]))");
            
        
                } // CALL | (prelude.ceu : lin 133 : col 14) | >=(i,#(
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 133 : col 24) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 133 : col 24) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 135 : col 16) | (set itr[:i] 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // SET | (prelude.ceu : lin 136 : col 13) | (set itr[:i] = 
                    
                { // CALL | (prelude.ceu : lin 136 : col 27) | +(i,1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local i */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 136, col 27) : {{+}}(i,1)");
            
        
                } // CALL | (prelude.ceu : lin 136 : col 27) | +(i,1)
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 136 : col 17) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 136, col 17) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 136, col 17) : itr[:i]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                // DCL | (prelude.ceu : lin 137 : col 13) | (val v = itr[:s
                
                    
                { // INDEX | (prelude.ceu : lin 137 : col 21) | itr[:s][i]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local i */));

                    
                    // COL
                    
                { // INDEX | (prelude.ceu : lin 137 : col 21) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 137, col 21) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 137, col 21) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 137, col 21) : itr[:s][i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 137, col 21) : itr[:s][i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local v */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 137 : col 13) | (val v = itr[:s)
        
            
                { // BLOCK | (prelude.ceu : lin 144 : col 21) | do (val ceu_3
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 145 : col 25) | (val ceu_3917 =
                
                    
                { // INDEX | (prelude.ceu : lin 138 : col 17) | itr[:tp]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=2} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 138, col 17) : itr[:tp]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 138, col 17) : itr[:tp]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local ceu_3917 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 145 : col 25) | (val ceu_3917 =)
        
            
                // DCL | (prelude.ceu : lin 140 : col 29) | (val it = ceu_3
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_3917 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 140 : col 29) | (val it = ceu_3)
        
            
                { // IF | (prelude.ceu : lin 139 : col 29) | if ==(it,:a
                    
                { // CALL | (prelude.ceu : lin 139 : col 20) | ==(it,:all)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_all} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 139, col 20) : {{==}}(it,:all)");
            
        
                } // CALL | (prelude.ceu : lin 139 : col 20) | ==(it,:all)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 139 : col 26) | [i,v]
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // TUPLE | (prelude.ceu : lin 139 : col 28) | [i,v]
                    ceux_push(X->S, 1, ceu_create_tuple(2));
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local i */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 139 : col 26) | [i,v]
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 141 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 143 : col 29) | (val it = ceu_3
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_3917 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 143 : col 29) | (val it = ceu_3)
        
            
                { // IF | (prelude.ceu : lin 140 : col 29) | if ==(it,:i
                    
                { // CALL | (prelude.ceu : lin 140 : col 20) | ==(it,:idx)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_idx} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 140, col 20) : {{==}}(it,:idx)");
            
        
                } // CALL | (prelude.ceu : lin 140 : col 20) | ==(it,:idx)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 140 : col 26) | i
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local i */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 140 : col 26) | i
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 142 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 144 : col 29) | (val it = ceu_3
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_3917 */));

                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 144 : col 29) | (val it = ceu_3)
        
            
                { // IF | (prelude.ceu : lin 141 : col 29) | if ==(it,:v
                    
                { // CALL | (prelude.ceu : lin 141 : col 20) | ==(it,:val)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 5 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_val} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 141, col 20) : {{==}}(it,:val)");
            
        
                } // CALL | (prelude.ceu : lin 141 : col 20) | ==(it,:val)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 141 : col 26) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 141 : col 26) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 143 : col 36) | if true v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 146 : col 29) | if true v e
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 142 : col 22) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 142 : col 22) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 144 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 144 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 144 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 143 : col 36) | if true v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 142 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 141 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 144 : col 21) | do (val ceu_3
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 135 : col 16) | (set itr[:i] 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 131 : col 37) | (val i = itr[
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 147 : col 5) | (func (itr :Ite
                    int ceu_f_iter_dash_dict (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 147 : col 36) | (val k = next
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 148 : col 9) | (val k = next-d
                
                    
                { // CALL | (prelude.ceu : lin 148 : col 17) | next-dict(itr[:
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 2) /* global next-dict */));

                    
                        
                { // INDEX | (prelude.ceu : lin 148 : col 27) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 148, col 27) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 148, col 27) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        
                { // INDEX | (prelude.ceu : lin 148 : col 33) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 148, col 33) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 148, col 33) : itr[:i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 148, col 17) : next-dict(itr[:s],itr[:i])");
            
        
                } // CALL | (prelude.ceu : lin 148 : col 17) | next-dict(itr[:
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local k */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 148 : col 9) | (val k = next-d)
        
            
                { // IF | (prelude.ceu : lin 149 : col 9) | if ==(k,nil
                    
                { // CALL | (prelude.ceu : lin 149 : col 14) | ==(k,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 149, col 14) : {{==}}(k,nil)");
            
        
                } // CALL | (prelude.ceu : lin 149 : col 14) | ==(k,nil)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 149 : col 21) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 149 : col 21) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 151 : col 16) | (set itr[:i] 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // SET | (prelude.ceu : lin 152 : col 13) | (set itr[:i] = 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));
  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 152 : col 17) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 152, col 17) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 152, col 17) : itr[:i]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                // DCL | (prelude.ceu : lin 153 : col 13) | (val v = itr[:s
                
                    
                { // INDEX | (prelude.ceu : lin 153 : col 21) | itr[:s][k]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));

                    
                    // COL
                    
                { // INDEX | (prelude.ceu : lin 153 : col 21) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 153, col 21) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 153, col 21) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 153, col 21) : itr[:s][k]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 153, col 21) : itr[:s][k]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local v */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 153 : col 13) | (val v = itr[:s)
        
            
                { // BLOCK | (prelude.ceu : lin 160 : col 21) | do (val ceu_4
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 161 : col 25) | (val ceu_4438 =
                
                    
                { // INDEX | (prelude.ceu : lin 154 : col 17) | itr[:tp]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=2} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 154, col 17) : itr[:tp]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 154, col 17) : itr[:tp]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local ceu_4438 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 161 : col 25) | (val ceu_4438 =)
        
            
                // DCL | (prelude.ceu : lin 156 : col 29) | (val it = ceu_4
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_4438 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 156 : col 29) | (val it = ceu_4)
        
            
                { // IF | (prelude.ceu : lin 155 : col 29) | if ==(it,:a
                    
                { // CALL | (prelude.ceu : lin 155 : col 20) | ==(it,:all)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_all} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 155, col 20) : {{==}}(it,:all)");
            
        
                } // CALL | (prelude.ceu : lin 155 : col 20) | ==(it,:all)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 155 : col 26) | [k,v]
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // TUPLE | (prelude.ceu : lin 155 : col 28) | [k,v]
                    ceux_push(X->S, 1, ceu_create_tuple(2));
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 155 : col 26) | [k,v]
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 157 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 159 : col 29) | (val it = ceu_4
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_4438 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 159 : col 29) | (val it = ceu_4)
        
            
                { // IF | (prelude.ceu : lin 156 : col 29) | if ==(it,:k
                    
                { // CALL | (prelude.ceu : lin 156 : col 20) | ==(it,:key)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_key} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 156, col 20) : {{==}}(it,:key)");
            
        
                } // CALL | (prelude.ceu : lin 156 : col 20) | ==(it,:key)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 156 : col 26) | k
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 156 : col 26) | k
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 158 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 160 : col 29) | (val it = ceu_4
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local ceu_4438 */));

                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 160 : col 29) | (val it = ceu_4)
        
            
                { // IF | (prelude.ceu : lin 157 : col 29) | if ==(it,:v
                    
                { // CALL | (prelude.ceu : lin 157 : col 20) | ==(it,:val)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 5 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_val} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 157, col 20) : {{==}}(it,:val)");
            
        
                } // CALL | (prelude.ceu : lin 157 : col 20) | ==(it,:val)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 157 : col 26) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 157 : col 26) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 159 : col 36) | if true k
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 162 : col 29) | if true k e
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 158 : col 22) | k
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local k */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 158 : col 22) | k
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 160 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 160 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 160 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 159 : col 36) | if true k
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 158 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 157 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 160 : col 21) | do (val ceu_4
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 151 : col 16) | (set itr[:i] 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 147 : col 36) | (val k = next
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 163 : col 5) | (func (itr :Ite
                    int ceu_f_iter_dash_xcoro (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 163 : col 37) | (val co = itr
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 164 : col 9) | (val co = itr[:
                
                    
                { // INDEX | (prelude.ceu : lin 164 : col 18) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 164, col 18) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 164, col 18) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local co */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 164 : col 9) | (val co = itr[:)
        
            
                { // BLOCK | (prelude.ceu : lin 170 : col 25) | do (val it = 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 165 : col 28) | (val it = (resu
                
                    
                ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local co */));

                ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                ceux_resume(X, 1 /* TODO: MULTI */, 1, CEU_ACTION_RESUME CEU4(COMMA X->now));
                
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 165, col 10) : (resume (co)(nil))");
            
        
            
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 165 : col 28) | (val it = (resu)
        
            
                { // IF | (prelude.ceu : lin 166 : col 13) | if ==(statu
                    
                { // CALL | (prelude.ceu : lin 166 : col 28) | ==(status(c
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 166 : col 17) | status(co)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 17) /* global status */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local co */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 166, col 17) : status(co)");
            
        
                } // CALL | (prelude.ceu : lin 166 : col 17) | status(co)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_terminated} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 166, col 28) : {{==}}(status(co),:terminated)");
            
        
                } // CALL | (prelude.ceu : lin 166 : col 28) | ==(status(c
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 167 : col 17) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 167 : col 17) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 168 : col 17) | it
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local it */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 168 : col 17) | it
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 170 : col 25) | do (val it = 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 163 : col 37) | (val co = itr
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 172 : col 5) | (func (itr :Ite
                    int ceu_f_iter_dash_coro (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 172 : col 36) | (set itr[:s] 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // SET | (prelude.ceu : lin 173 : col 9) | (set itr[:s] = 
                    
                { // CALL | (prelude.ceu : lin 173 : col 21) | coroutine(itr[:
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 16) /* global coroutine */));

                    
                        
                { // INDEX | (prelude.ceu : lin 173 : col 31) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 173, col 31) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 173, col 31) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 173, col 21) : coroutine(itr[:s])");
            
        
                } // CALL | (prelude.ceu : lin 173 : col 21) | coroutine(itr[:
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 173 : col 13) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 173, col 13) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 173, col 13) : itr[:s]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                { // SET | (prelude.ceu : lin 174 : col 9) | (set itr[:f] = 
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 40) /* global iter-xcoro */));
  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 174 : col 13) | itr[:f]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 174, col 13) : itr[:f]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 174, col 13) : itr[:f]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                { // CALL | (prelude.ceu : lin 175 : col 9) | itr[:f](itr)
                    
                { // INDEX | (prelude.ceu : lin 175 : col 9) | itr[:f]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 175, col 9) : itr[:f]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 175, col 9) : itr[:f]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 175, col 9) : itr[:f](itr)");
            
        
                } // CALL | (prelude.ceu : lin 175 : col 9) | itr[:f](itr)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 172 : col 36) | (set itr[:s] 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 178 : col 5) | (func (itr :Ite
                    int ceu_f_iter_dash_tasks (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 178 : col 37) | (set itr[:i] 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // SET | (prelude.ceu : lin 180 : col 9) | (set itr[:i] = 
                    
                { // CALL | (prelude.ceu : lin 180 : col 21) | next-tasks(itr[
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 19) /* global next-tasks */));

                    
                        
                { // INDEX | (prelude.ceu : lin 180 : col 32) | itr[:s]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 180, col 32) : itr[:s]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 180, col 32) : itr[:s]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        
                { // INDEX | (prelude.ceu : lin 180 : col 38) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 180, col 38) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 180, col 38) : itr[:i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 180, col 21) : next-tasks(itr[:s],itr[:i])");
            
        
                } // CALL | (prelude.ceu : lin 180 : col 21) | next-tasks(itr[
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 180 : col 13) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 180, col 13) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 180, col 13) : itr[:i]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                { // INDEX | (prelude.ceu : lin 181 : col 9) | itr[:i]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=3} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg itr */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 181, col 9) : itr[:i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 181, col 9) : itr[:i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 178 : col 37) | (set itr[:i] 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 184 : col 5) | (func (v,tp) 
                    int ceu_f_to_dash_iter (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 184 : col 26) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 196 : col 21) | do (val ceu_5
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 197 : col 25) | (val ceu_5651 =
                
                    
                { // CALL | (prelude.ceu : lin 185 : col 13) | type(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 185, col 13) : type(v)");
            
        
                } // CALL | (prelude.ceu : lin 185 : col 13) | type(v)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 197 : col 25) | (val ceu_5651 =)
        
            
                // DCL | (prelude.ceu : lin 187 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 187 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 186 : col 29) | if tags(v,:Iter
                    
                { // CALL | (prelude.ceu : lin 186 : col 14) | tags(v,:Iterato
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 186, col 14) : tags(v,:Iterator)");
            
        
                } // CALL | (prelude.ceu : lin 186 : col 14) | tags(v,:Iterato
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 186 : col 33) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 186 : col 33) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 188 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 190 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 190 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 187 : col 29) | if ==(it,:t
                    
                { // CALL | (prelude.ceu : lin 187 : col 15) | ==(it,:tupl
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tuple} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 187, col 15) : {{==}}(it,:tuple)");
            
        
                } // CALL | (prelude.ceu : lin 187 : col 15) | ==(it,:tupl
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 187 : col 23) | tags([iter-tu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 187 : col 28) | tags([iter-tupl
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 187 : col 38) | [iter-tuple,v,t
                    ceux_push(X->S, 1, ceu_create_tuple(4));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 38) /* global iter-tuple */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg tp */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 187, col 28) : tags([iter-tuple,v,tp,0],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 187 : col 28) | tags([iter-tupl
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 187 : col 23) | tags([iter-tu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 189 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 191 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 191 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 188 : col 29) | if ==(it,:v
                    
                { // CALL | (prelude.ceu : lin 188 : col 15) | ==(it,:vect
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 188, col 15) : {{==}}(it,:vector)");
            
        
                } // CALL | (prelude.ceu : lin 188 : col 15) | ==(it,:vect
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 188 : col 24) | tags([iter-tu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 188 : col 28) | tags([iter-tupl
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 188 : col 38) | [iter-tuple,v,t
                    ceux_push(X->S, 1, ceu_create_tuple(4));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 38) /* global iter-tuple */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg tp */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 188, col 28) : tags([iter-tuple,v,tp,0],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 188 : col 28) | tags([iter-tupl
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 188 : col 24) | tags([iter-tu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 190 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 192 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 192 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 189 : col 29) | if ==(it,:d
                    
                { // CALL | (prelude.ceu : lin 189 : col 15) | ==(it,:dict
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_dict} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 189, col 15) : {{==}}(it,:dict)");
            
        
                } // CALL | (prelude.ceu : lin 189 : col 15) | ==(it,:dict
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 189 : col 22) | tags([iter-di
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 189 : col 28) | tags([iter-dict
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 189 : col 38) | [iter-dict,v,tp
                    ceux_push(X->S, 1, ceu_create_tuple(4));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 39) /* global iter-dict */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg tp */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 189, col 28) : tags([iter-dict,v,tp,nil],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 189 : col 28) | tags([iter-dict
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 189 : col 22) | tags([iter-di
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 191 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 193 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 193 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 190 : col 29) | if ==(it,:f
                    
                { // CALL | (prelude.ceu : lin 190 : col 15) | ==(it,:func
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 5 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_func} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 190, col 15) : {{==}}(it,:func)");
            
        
                } // CALL | (prelude.ceu : lin 190 : col 15) | ==(it,:func
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 190 : col 22) | tags([v],:Ite
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 190 : col 28) | tags([v],:Itera
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 190 : col 38) | [v]
                    ceux_push(X->S, 1, ceu_create_tuple(1));
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 190, col 28) : tags([v],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 190 : col 28) | tags([v],:Itera
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 190 : col 22) | tags([v],:Ite
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 192 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 194 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 6 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 194 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 191 : col 29) | if ==(it,:c
                    
                { // CALL | (prelude.ceu : lin 191 : col 15) | ==(it,:coro
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 6 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_coro} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 191, col 15) : {{==}}(it,:coro)");
            
        
                } // CALL | (prelude.ceu : lin 191 : col 15) | ==(it,:coro
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 191 : col 22) | tags([iter-co
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 191 : col 28) | tags([iter-coro
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 191 : col 38) | [iter-coro,v]
                    ceux_push(X->S, 1, ceu_create_tuple(2));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 41) /* global iter-coro */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 191, col 28) : tags([iter-coro,v],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 191 : col 28) | tags([iter-coro
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 191 : col 22) | tags([iter-co
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 193 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 195 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 7 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 195 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 192 : col 29) | if ==(it,:e
                    
                { // CALL | (prelude.ceu : lin 192 : col 15) | ==(it,:exe-
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 7 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_exe_coro} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 192, col 15) : {{==}}(it,:exe-coro)");
            
        
                } // CALL | (prelude.ceu : lin 192 : col 15) | ==(it,:exe-
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 192 : col 26) | tags([iter-xc
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+8, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 192 : col 28) | tags([iter-xcor
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 192 : col 38) | [iter-xcoro,v]
                    ceux_push(X->S, 1, ceu_create_tuple(2));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 40) /* global iter-xcoro */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 192, col 28) : tags([iter-xcoro,v],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 192 : col 28) | tags([iter-xcor
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 192 : col 26) | tags([iter-xc
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+8, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 194 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+8, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 196 : col 29) | (val it = ceu_5
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_5651 */));

                    ceux_copy(X->S, (X->base + 0 + 8 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 196 : col 29) | (val it = ceu_5)
        
            
                { // IF | (prelude.ceu : lin 193 : col 29) | if ==(it,:t
                    
                { // CALL | (prelude.ceu : lin 193 : col 15) | ==(it,:task
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 8 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tasks} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 193, col 15) : {{==}}(it,:tasks)");
            
        
                } // CALL | (prelude.ceu : lin 193 : col 15) | ==(it,:task
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 193 : col 23) | tags([iter-ta
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+9, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 193 : col 28) | tags([iter-task
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        
                { // TUPLE | (prelude.ceu : lin 193 : col 38) | [iter-tasks,v,n
                    ceux_push(X->S, 1, ceu_create_tuple(4));
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 42) /* global iter-tasks */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        ceu_tuple_set(&ceux_peek(X->S,XX(-2)).Dyn->Tuple, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iterator} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                                        
                    

                    ceux_call(X, 3, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 193, col 28) : tags([iter-tasks,v,nil,nil],:Iterator,true)");
            
        
                } // CALL | (prelude.ceu : lin 193 : col 28) | tags([iter-task
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 193 : col 23) | tags([iter-ta
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+9, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 195 : col 36) | if true nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+9, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 198 : col 29) | if true nil
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 194 : col 18) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+9, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 194 : col 18) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+9, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 196 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+9, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 196 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 196 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+9, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 195 : col 36) | if true nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+9, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 194 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+8, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 193 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 192 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 191 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 190 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 189 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 188 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 196 : col 21) | do (val ceu_5
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 184 : col 26) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 223 : col 9) | (func (v1',v2')
                    int ceu_f_equals_dash_tuple_dash_vector (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 223 : col 45) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 229 : col 21) | do (val ceu_6
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 230 : col 25) | (val ceu_6910 =
                
                    
                { // CALL | (prelude.ceu : lin 224 : col 19) | ==(#(v1
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 224 : col 14) | #(v1')
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1' */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 224, col 14) : {{#}}(v1')");
            
        
                } // CALL | (prelude.ceu : lin 224 : col 14) | #(v1')
                
                    
                        
                { // CALL | (prelude.ceu : lin 224 : col 22) | #(v2')
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2' */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 224, col 22) : {{#}}(v2')");
            
        
                } // CALL | (prelude.ceu : lin 224 : col 22) | #(v2')
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 224, col 19) : {{==}}({{#}}(v1'),{{#}}(v2'))");
            
        
                } // CALL | (prelude.ceu : lin 224 : col 19) | ==(#(v1
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_6910 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 230 : col 25) | (val ceu_6910 =)
        
            
                { // IF | (prelude.ceu : lin 225 : col 25) | if ceu_6910 i
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_6910 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 225 : col 37) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 224 : col 33) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 228 : col 29) | do (val ceu_7
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 229 : col 33) | (val ceu_7030 =
                
                    
                { // CALL | (prelude.ceu : lin 229 : col 48) | to-iter(to-iter
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        
                { // CALL | (prelude.ceu : lin 225 : col 27) | to-iter(v1',:al
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1' */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_all} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 225, col 27) : to-iter(v1',:all)");
            
        
                } // CALL | (prelude.ceu : lin 225 : col 27) | to-iter(v1',:al
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 229, col 48) : to-iter(to-iter(v1',:all))");
            
        
                } // CALL | (prelude.ceu : lin 229 : col 48) | to-iter(to-iter
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_7030 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 229 : col 33) | (val ceu_7030 =)
        
            
                // LOOP | (prelude.ceu : lin 226 : col 33) | loop (val x =
                CEU_LOOP_START_8874:
                    
                { // BLOCK | (prelude.ceu : lin 226 : col 33) | (val x = ceu_
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 227 : col 37) | (val x = ceu_70
                
                    
                { // CALL | (prelude.ceu : lin 225 : col 26) | ceu_7030[0](ceu
                    
                { // INDEX | (prelude.ceu : lin 225 : col 26) | ceu_7030[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_7030 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 225, col 26) : ceu_7030[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 225, col 26) : ceu_7030[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_7030 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 225, col 26) : ceu_7030[0](ceu_7030)");
            
        
                } // CALL | (prelude.ceu : lin 225 : col 26) | ceu_7030[0](ceu
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local x */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 227 : col 37) | (val x = ceu_70)
        
             // BREAK | (prelude.ceu : lin 226 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 226 : col 56) | ==(x,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local x */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 226, col 56) : {{==}}(x,nil)");
            
        
                } // CALL | (prelude.ceu : lin 226 : col 56) | ==(x,nil)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_8874;
                    }
                }
             // BREAK | (prelude.ceu : lin 226 : col 21) | (break if if 
                
                { // IF | (prelude.ceu : lin 226 : col 47) | if ===(v2'[
                    
                { // CALL | (prelude.ceu : lin 226 : col 38) | ===(v2'[x[0
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 44) /* global {{===}} */));

                    
                        
                { // INDEX | (prelude.ceu : lin 226 : col 28) | v2'[x[0]]
                    // IDX
                    
                { // INDEX | (prelude.ceu : lin 226 : col 32) | x[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local x */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 226, col 32) : x[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 226, col 32) : x[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2' */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 226, col 28) : v2'[x[0]]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 226, col 28) : v2'[x[0]]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        
                { // INDEX | (prelude.ceu : lin 226 : col 42) | x[1]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local x */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 226, col 42) : x[1]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 226, col 42) : x[1]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 226, col 38) : {{===}}(v2'[x[0]],x[1])");
            
        
                } // CALL | (prelude.ceu : lin 226 : col 38) | ===(v2'[x[0
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 226 : col 48) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 226 : col 48) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 226 : col 63) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 226 : col 63) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_8874;
                    }
                }
            
                        
                            CEU_LOOP_STOP_8874:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 226 : col 33) | (val x = ceu_
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_8874;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 228 : col 29) | do (val ceu_7
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 229 : col 3) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 229 : col 3) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 229 : col 18) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 229 : col 18) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 225 : col 37) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 231 : col 32) | ceu_6910
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_6910 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 231 : col 32) | ceu_6910
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 229 : col 21) | do (val ceu_6
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 223 : col 45) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 230 : col 9) | (func (v1',v2')
                    int ceu_f_contains_dash_dict_question_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 230 : col 40) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 231 : col 14) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 235 : col 29) | do (val ceu_7
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 236 : col 33) | (val ceu_7756 =
                
                    
                { // CALL | (prelude.ceu : lin 236 : col 48) | to-iter(to-iter
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        
                { // CALL | (prelude.ceu : lin 232 : col 27) | to-iter(v1',:al
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1' */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_all} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 232, col 27) : to-iter(v1',:all)");
            
        
                } // CALL | (prelude.ceu : lin 232 : col 27) | to-iter(v1',:al
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 236, col 48) : to-iter(to-iter(v1',:all))");
            
        
                } // CALL | (prelude.ceu : lin 236 : col 48) | to-iter(to-iter
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_7756 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 236 : col 33) | (val ceu_7756 =)
        
            
                // LOOP | (prelude.ceu : lin 233 : col 33) | loop (val x =
                CEU_LOOP_START_9029:
                    
                { // BLOCK | (prelude.ceu : lin 233 : col 33) | (val x = ceu_
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 234 : col 37) | (val x = ceu_77
                
                    
                { // CALL | (prelude.ceu : lin 232 : col 26) | ceu_7756[0](ceu
                    
                { // INDEX | (prelude.ceu : lin 232 : col 26) | ceu_7756[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_7756 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 232, col 26) : ceu_7756[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 232, col 26) : ceu_7756[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_7756 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 232, col 26) : ceu_7756[0](ceu_7756)");
            
        
                } // CALL | (prelude.ceu : lin 232 : col 26) | ceu_7756[0](ceu
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local x */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 234 : col 37) | (val x = ceu_77)
        
             // BREAK | (prelude.ceu : lin 233 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 233 : col 56) | ==(x,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local x */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 233, col 56) : {{==}}(x,nil)");
            
        
                } // CALL | (prelude.ceu : lin 233 : col 56) | ==(x,nil)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_9029;
                    }
                }
             // BREAK | (prelude.ceu : lin 233 : col 21) | (break if if 
                
                { // IF | (prelude.ceu : lin 233 : col 47) | if ===(v2'[
                    
                { // CALL | (prelude.ceu : lin 233 : col 38) | ===(v2'[x[0
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 44) /* global {{===}} */));

                    
                        
                { // INDEX | (prelude.ceu : lin 233 : col 28) | v2'[x[0]]
                    // IDX
                    
                { // INDEX | (prelude.ceu : lin 233 : col 32) | x[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local x */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 233, col 32) : x[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 233, col 32) : x[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2' */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 233, col 28) : v2'[x[0]]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 233, col 28) : v2'[x[0]]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        
                { // INDEX | (prelude.ceu : lin 233 : col 42) | x[1]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local x */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 233, col 42) : x[1]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 233, col 42) : x[1]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 233, col 38) : {{===}}(v2'[x[0]],x[1])");
            
        
                } // CALL | (prelude.ceu : lin 233 : col 38) | ===(v2'[x[0
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 233 : col 48) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 233 : col 48) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 233 : col 63) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 233 : col 63) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_9029;
                    }
                }
            
                        
                            CEU_LOOP_STOP_9029:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 233 : col 33) | (val x = ceu_
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_9029;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 235 : col 29) | do (val ceu_7
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 236 : col 3) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 236 : col 3) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 236 : col 18) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 236 : col 18) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 230 : col 40) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 213 : col 1) | (func :rec (v1,
                    int ceu_f_equals_equals_equals (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 213 : col 27) | (val t1 = typ
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 214 : col 5) | (val t1 = type(
                
                    
                { // CALL | (prelude.ceu : lin 214 : col 14) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 214, col 14) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 214 : col 14) | type(v1)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local t1 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 214 : col 5) | (val t1 = type()
        
            
                // DCL | (prelude.ceu : lin 215 : col 5) | (val t2 = type(
                
                    
                { // CALL | (prelude.ceu : lin 215 : col 14) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 215, col 14) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 215 : col 14) | type(v2)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local t2 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 215 : col 5) | (val t2 = type()
        
            
                { // BLOCK | (prelude.ceu : lin 222 : col 7) | do (val equal
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 223 : col 9) | (val equals-tup
                
                     // CREATE | (prelude.ceu : lin 223 : col 9) | (func (v1',v2')
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_equals_dash_tuple_dash_vector,
                        2,  // TODO: remove assert
                        3,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local equals-tuple-vector */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 223 : col 9) | (val equals-tup)
        
            
                // DCL | (prelude.ceu : lin 230 : col 9) | (val contains-d
                
                     // CREATE | (prelude.ceu : lin 230 : col 9) | (func (v1',v2')
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_contains_dash_dict_question_,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 1) /* local contains-dict? */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 230 : col 9) | (val contains-d)
        
            
                { // BLOCK | (prelude.ceu : lin 223 : col 21) | do if ==(
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 227 : col 29) | if ==(v1,v2
                    
                { // CALL | (prelude.ceu : lin 217 : col 12) | ==(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 217, col 12) : {{==}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 217 : col 12) | ==(v1,v2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 217 : col 19) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 217 : col 19) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 219 : col 36) | if /=(t1,
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 222 : col 29) | if /=(t1,t2
                    
                { // CALL | (prelude.ceu : lin 218 : col 12) | /=(t1,t2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 15) /* global {{/=}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local t1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local t2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 218, col 12) : {{/=}}(t1,t2)");
            
        
                } // CALL | (prelude.ceu : lin 218 : col 12) | /=(t1,t2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 218 : col 19) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 218 : col 19) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 220 : col 36) | if do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 223 : col 29) | if do (val ce
                    
                { // BLOCK | (prelude.ceu : lin 220 : col 21) | do (val ceu_6
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 221 : col 25) | (val ceu_6538 =
                
                    
                { // CALL | (prelude.ceu : lin 219 : col 12) | ==(t1,:tupl
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local t1 */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tuple} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 219, col 12) : {{==}}(t1,:tuple)");
            
        
                } // CALL | (prelude.ceu : lin 219 : col 12) | ==(t1,:tupl
                
                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local ceu_6538 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 221 : col 25) | (val ceu_6538 =)
        
            
                { // IF | (prelude.ceu : lin 220 : col 25) | if ceu_6538 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local ceu_6538 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 220 : col 37) | ceu_6538
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local ceu_6538 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 220 : col 37) | ceu_6538
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 222 : col 32) | ==(t1,:ve
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 219 : col 28) | ==(t1,:vect
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local t1 */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 219, col 28) : {{==}}(t1,:vector)");
            
        
                } // CALL | (prelude.ceu : lin 219 : col 28) | ==(t1,:vect
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 222 : col 32) | ==(t1,:ve
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 220 : col 21) | do (val ceu_6
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 221 : col 3) | equals-tuple-
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 219 : col 42) | equals-tuple-ve
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local equals-tuple-vector */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 219, col 42) : equals-tuple-vector(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 219 : col 42) | equals-tuple-ve
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 3) | equals-tuple-
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 221 : col 36) | if ==(t1,
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 224 : col 29) | if ==(t1,:d
                    
                { // CALL | (prelude.ceu : lin 220 : col 12) | ==(t1,:dict
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local t1 */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_dict} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 220, col 12) : {{==}}(t1,:dict)");
            
        
                } // CALL | (prelude.ceu : lin 220 : col 12) | ==(t1,:dict
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 220 : col 22) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 221 : col 21) | do (val ceu_6
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 222 : col 25) | (val ceu_6626 =
                
                    
                { // CALL | (prelude.ceu : lin 220 : col 24) | contains-dict?(
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 1) /* local contains-dict? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 220, col 24) : contains-dict?(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 220 : col 24) | contains-dict?(
                
                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local ceu_6626 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 222 : col 25) | (val ceu_6626 =)
        
            
                { // IF | (prelude.ceu : lin 221 : col 25) | if ceu_6626 c
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local ceu_6626 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 221 : col 37) | contains-dict
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 220 : col 50) | contains-dict?(
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 1) /* local contains-dict? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 220, col 50) : contains-dict?(v2,v1)");
            
        
                } // CALL | (prelude.ceu : lin 220 : col 50) | contains-dict?(
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 37) | contains-dict
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 221 : col 32) | ceu_6626
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local ceu_6626 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 32) | ceu_6626
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 21) | do (val ceu_6
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 220 : col 22) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 226 : col 36) | if true fal
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 229 : col 29) | if true false
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 221 : col 14) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 14) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 223 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 223 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 223 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 226 : col 36) | if true fal
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 221 : col 36) | if ==(t1,
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 220 : col 36) | if do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 219 : col 36) | if /=(t1,
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 223 : col 21) | do if ==(
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 222 : col 7) | do (val equal
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 213 : col 27) | (val t1 = typ
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 240 : col 1) | (func :rec (v1,
                    int ceu_f_equals_slash_equals (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 240 : col 27) | if ===(v1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 241 : col 6) | if ===(v1,v
                    
                { // CALL | (prelude.ceu : lin 241 : col 13) | ===(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 44) /* global {{===}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 241, col 13) : {{===}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 241 : col 13) | ===(v1,v2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 241 : col 21) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 241 : col 21) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 241 : col 36) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 241 : col 36) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 240 : col 27) | if ===(v1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 244 : col 1) | (func (v1,v2) 
                    int ceu_f_is_plic_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 244 : col 18) | do if ==
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 252 : col 21) | do if ==(
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 256 : col 29) | if ==(v1,v2
                    
                { // CALL | (prelude.ceu : lin 246 : col 13) | ==(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 246, col 13) : {{==}}(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 246 : col 13) | ==(v1,v2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 246 : col 20) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 246 : col 20) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 248 : col 36) | if =/=(ty
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 251 : col 29) | if =/=(type
                    
                { // CALL | (prelude.ceu : lin 247 : col 19) | =/=(type(v2
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 45) /* global {{=/=}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 247 : col 10) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 247, col 10) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 247 : col 10) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 247, col 19) : {{=/=}}(type(v2),:tag)");
            
        
                } // CALL | (prelude.ceu : lin 247 : col 19) | =/=(type(v2
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 247 : col 29) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 247 : col 29) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 249 : col 36) | if ===(ty
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 252 : col 29) | if ===(type
                    
                { // CALL | (prelude.ceu : lin 248 : col 19) | ===(type(v1
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 44) /* global {{===}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 248 : col 10) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 248, col 10) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 248 : col 10) | type(v1)
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 248, col 19) : {{===}}(type(v1),v2)");
            
        
                } // CALL | (prelude.ceu : lin 248 : col 19) | ===(type(v1
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 248 : col 27) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 248 : col 27) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 250 : col 36) | if tags(v1,v2
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 253 : col 29) | if tags(v1,v2) 
                    
                { // CALL | (prelude.ceu : lin 249 : col 9) | tags(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 6) /* global tags */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 249, col 9) : tags(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 249 : col 9) | tags(v1,v2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 249 : col 21) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 249 : col 21) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 251 : col 36) | if true fal
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 254 : col 29) | if true false
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 250 : col 14) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 250 : col 14) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 252 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 252 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 252 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 251 : col 36) | if true fal
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 250 : col 36) | if tags(v1,v2
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 249 : col 36) | if ===(ty
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 248 : col 36) | if =/=(ty
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 252 : col 21) | do if ==(
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 244 : col 18) | do if ==
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 254 : col 1) | (func (v1,v2) 
                    int ceu_f_is_dash_not_plic_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 254 : col 22) | if is'(v1,v2)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 255 : col 6) | if is'(v1,v2) 
                    
                { // CALL | (prelude.ceu : lin 255 : col 9) | is'(v1,v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 255, col 9) : is'(v1,v2)");
            
        
                } // CALL | (prelude.ceu : lin 255 : col 9) | is'(v1,v2)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 255 : col 20) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 255 : col 20) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 255 : col 35) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 255 : col 35) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 254 : col 22) | if is'(v1,v2)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 258 : col 1) | (func (v,xs) 
                    int ceu_f_in_plic_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 258 : col 18) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 262 : col 29) | do (val ceu_9
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 263 : col 33) | (val ceu_9812 =
                
                    
                { // CALL | (prelude.ceu : lin 263 : col 48) | to-iter(to-iter
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        
                { // CALL | (prelude.ceu : lin 259 : col 15) | to-iter(xs)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg xs */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 259, col 15) : to-iter(xs)");
            
        
                } // CALL | (prelude.ceu : lin 259 : col 15) | to-iter(xs)
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 263, col 48) : to-iter(to-iter(xs))");
            
        
                } // CALL | (prelude.ceu : lin 263 : col 48) | to-iter(to-iter
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_9812 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 263 : col 33) | (val ceu_9812 =)
        
            
                // LOOP | (prelude.ceu : lin 260 : col 33) | loop (val x =
                CEU_LOOP_START_9984:
                    
                { // BLOCK | (prelude.ceu : lin 260 : col 33) | (val x = ceu_
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 261 : col 37) | (val x = ceu_98
                
                    
                { // CALL | (prelude.ceu : lin 259 : col 14) | ceu_9812[0](ceu
                    
                { // INDEX | (prelude.ceu : lin 259 : col 14) | ceu_9812[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_9812 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 259, col 14) : ceu_9812[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 259, col 14) : ceu_9812[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_9812 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 259, col 14) : ceu_9812[0](ceu_9812)");
            
        
                } // CALL | (prelude.ceu : lin 259 : col 14) | ceu_9812[0](ceu
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local x */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 261 : col 37) | (val x = ceu_98)
        
             // BREAK | (prelude.ceu : lin 260 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 260 : col 56) | ==(x,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local x */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 260, col 56) : {{==}}(x,nil)");
            
        
                } // CALL | (prelude.ceu : lin 260 : col 56) | ==(x,nil)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_9984;
                    }
                }
             // BREAK | (prelude.ceu : lin 260 : col 8) | (break if ==
                
                { // CALL | (prelude.ceu : lin 260 : col 17) | ==(x,v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local x */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 260, col 17) : {{==}}(x,v)");
            
        
                } // CALL | (prelude.ceu : lin 260 : col 17) | ==(x,v)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_9984;
                    }
                }
            
                        
                            CEU_LOOP_STOP_9984:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 260 : col 33) | (val x = ceu_
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_9984;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 262 : col 29) | do (val ceu_9
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 258 : col 18) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 264 : col 1) | (func (v,xs) 
                    int ceu_f_in_dash_not_plic_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 264 : col 22) | if in'(v,xs) 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 265 : col 6) | if in'(v,xs) 
                    
                { // CALL | (prelude.ceu : lin 265 : col 9) | in'(v,xs)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 48) /* global in' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg xs */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 265, col 9) : in'(v,xs)");
            
        
                } // CALL | (prelude.ceu : lin 265 : col 9) | in'(v,xs)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 265 : col 19) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 265 : col 19) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 265 : col 34) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 265 : col 34) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 264 : col 22) | if in'(v,xs) 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 272 : col 1) | (func (v) do 
                    int ceu_f_string_question_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 272 : col 18) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 274 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 275 : col 25) | (val ceu_10176 
                
                    
                { // BLOCK | (prelude.ceu : lin 274 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 275 : col 25) | (val ceu_10105 
                
                    
                { // CALL | (prelude.ceu : lin 273 : col 14) | ==(type(v),
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 273 : col 7) | type(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 7) : type(v)");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 7) | type(v)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 14) : {{==}}(type(v),:vector)");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 14) | ==(type(v),
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_10105 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 275 : col 25) | (val ceu_10105 )
        
            
                { // IF | (prelude.ceu : lin 274 : col 25) | if ceu_10105 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_10105 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 274 : col 38) | >(#(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 273 : col 32) | >(#(v),
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 273 : col 30) | #(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 30) : {{#}}(v)");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 30) | #(v)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 32) : {{>}}({{#}}(v),0)");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 32) | >(#(v),
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 274 : col 38) | >(#(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 274 : col 32) | ceu_10105
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_10105 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 274 : col 32) | ceu_10105
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 274 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_10176 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 275 : col 25) | (val ceu_10176 )
        
            
                { // IF | (prelude.ceu : lin 278 : col 25) | if ceu_10176 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10176 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 278 : col 38) | ==(type(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 273 : col 52) | ==(type(v[0
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 273 : col 42) | type(v[0])
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        
                { // INDEX | (prelude.ceu : lin 273 : col 47) | v[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 273, col 47) : v[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 273, col 47) : v[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 42) : type(v[0])");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 42) | type(v[0])
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_char} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 273, col 52) : {{==}}(type(v[0]),:char)");
            
        
                } // CALL | (prelude.ceu : lin 273 : col 52) | ==(type(v[0
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 278 : col 38) | ==(type(v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 274 : col 32) | ceu_10176
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10176 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 274 : col 32) | ceu_10176
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 274 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 272 : col 18) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 276 : col 1) | (func :rec (v) 
                    int ceu_f_to_dash_string (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 276 : col 25) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 286 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 287 : col 25) | (val ceu_10612 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 287 : col 25) | (val ceu_10612 )
        
            
                // DCL | (prelude.ceu : lin 279 : col 29) | (val it = ceu_1
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 279 : col 29) | (val it = ceu_1)
        
            
                { // IF | (prelude.ceu : lin 278 : col 29) | if string?(it) 
                    
                { // CALL | (prelude.ceu : lin 278 : col 9) | string?(it)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 50) /* global string? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local it */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 278, col 9) : string?(it)");
            
        
                } // CALL | (prelude.ceu : lin 278 : col 9) | string?(it)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 278 : col 21) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 278 : col 21) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 280 : col 36) | (val it :nil 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 282 : col 29) | (val it :nil = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 282 : col 29) | (val it :nil = )
        
            
                { // IF | (prelude.ceu : lin 280 : col 29) | if is'(it,:nil)
                    
                { // CALL | (prelude.ceu : lin 279 : col 16) | is'(it,:nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_nil} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 279, col 16) : is'(it,:nil)");
            
        
                } // CALL | (prelude.ceu : lin 279 : col 16) | is'(it,:nil)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 279 : col 22) | #['n','i','l'
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // VECTOR | (prelude.ceu : lin 279 : col 24) | #['n','i','l']
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='n'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='i'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='l'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 279 : col 22) | #['n','i','l'
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 281 : col 36) | (val it :tag 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 283 : col 29) | (val it :tag = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 283 : col 29) | (val it :tag = )
        
            
                { // IF | (prelude.ceu : lin 281 : col 29) | if is'(it,:tag)
                    
                { // CALL | (prelude.ceu : lin 280 : col 16) | is'(it,:tag)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 280, col 16) : is'(it,:tag)");
            
        
                } // CALL | (prelude.ceu : lin 280 : col 16) | is'(it,:tag)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 280 : col 22) | to-string-tag
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 280 : col 24) | to-string-tag(v
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 9) /* global to-string-tag */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 280, col 24) : to-string-tag(v)");
            
        
                } // CALL | (prelude.ceu : lin 280 : col 24) | to-string-tag(v
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 280 : col 22) | to-string-tag
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 282 : col 36) | (val it :numb
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 284 : col 29) | (val it :number
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 284 : col 29) | (val it :number)
        
            
                { // IF | (prelude.ceu : lin 282 : col 29) | if is'(it,:numb
                    
                { // CALL | (prelude.ceu : lin 281 : col 16) | is'(it,:number)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 281, col 16) : is'(it,:number)");
            
        
                } // CALL | (prelude.ceu : lin 281 : col 16) | is'(it,:number)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 281 : col 25) | to-string-num
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 281 : col 24) | to-string-numbe
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 7) /* global to-string-number */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 281, col 24) : to-string-number(v)");
            
        
                } // CALL | (prelude.ceu : lin 281 : col 24) | to-string-numbe
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 281 : col 25) | to-string-num
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 283 : col 36) | (val it :char
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 285 : col 29) | (val it :char =
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 285 : col 29) | (val it :char =)
        
            
                { // IF | (prelude.ceu : lin 283 : col 29) | if is'(it,:char
                    
                { // CALL | (prelude.ceu : lin 282 : col 16) | is'(it,:char)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 5 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_char} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 282, col 16) : is'(it,:char)");
            
        
                } // CALL | (prelude.ceu : lin 282 : col 16) | is'(it,:char)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 282 : col 23) | #[v]
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // VECTOR | (prelude.ceu : lin 282 : col 24) | #[v]
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 282 : col 23) | #[v]
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 284 : col 36) | (val it :poin
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 286 : col 29) | (val it :pointe
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_10612 */));

                    ceux_copy(X->S, (X->base + 0 + 6 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 286 : col 29) | (val it :pointe)
        
            
                { // IF | (prelude.ceu : lin 284 : col 29) | if is'(it,:poin
                    
                { // CALL | (prelude.ceu : lin 283 : col 16) | is'(it,:pointer
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 6 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_pointer} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 283, col 16) : is'(it,:pointer)");
            
        
                } // CALL | (prelude.ceu : lin 283 : col 16) | is'(it,:pointer
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 283 : col 26) | to-string-poi
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 283 : col 24) | to-string-point
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 8) /* global to-string-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 283, col 24) : to-string-pointer(v)");
            
        
                } // CALL | (prelude.ceu : lin 283 : col 24) | to-string-point
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 283 : col 26) | to-string-poi
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 285 : col 36) | if true nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 288 : col 29) | if true nil
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 284 : col 14) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 284 : col 14) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 286 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 286 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 286 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 285 : col 36) | if true nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 284 : col 36) | (val it :poin
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 283 : col 36) | (val it :char
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 282 : col 36) | (val it :numb
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 281 : col 36) | (val it :tag 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 280 : col 36) | (val it :nil 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 286 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 276 : col 25) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 288 : col 1) | (func (v) if 
                    int ceu_f_to_dash_bool (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 288 : col 18) | if if v fal
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 289 : col 6) | if if v false
                    
                { // IF | (prelude.ceu : lin 289 : col 11) | if v false 
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 289 : col 16) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 289 : col 16) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 289 : col 31) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 289 : col 31) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 290 : col 3) | false
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 290 : col 3) | false
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 290 : col 18) | true
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 290 : col 18) | true
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 288 : col 18) | if if v fal
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 292 : col 1) | (func (v) do 
                    int ceu_f_to_dash_number (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 292 : col 20) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 299 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 300 : col 25) | (val ceu_11289 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_11289 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 300 : col 25) | (val ceu_11289 )
        
            
                // DCL | (prelude.ceu : lin 295 : col 29) | (val it :tag = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11289 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 295 : col 29) | (val it :tag = )
        
            
                { // IF | (prelude.ceu : lin 295 : col 29) | if is'(it,:tag)
                    
                { // CALL | (prelude.ceu : lin 294 : col 16) | is'(it,:tag)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 294, col 16) : is'(it,:tag)");
            
        
                } // CALL | (prelude.ceu : lin 294 : col 16) | is'(it,:tag)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 294 : col 22) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(   (ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Tag)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 294 : col 22) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 296 : col 36) | (val it :char
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 298 : col 29) | (val it :char =
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11289 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 298 : col 29) | (val it :char =)
        
            
                { // IF | (prelude.ceu : lin 296 : col 29) | if is'(it,:char
                    
                { // CALL | (prelude.ceu : lin 295 : col 16) | is'(it,:char)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_char} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 295, col 16) : is'(it,:char)");
            
        
                } // CALL | (prelude.ceu : lin 295 : col 16) | is'(it,:char)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 295 : col 23) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(   (ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Char)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 295 : col 23) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 297 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 299 : col 29) | (val it = ceu_1
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11289 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 299 : col 29) | (val it = ceu_1)
        
            
                { // IF | (prelude.ceu : lin 296 : col 29) | if string?(it) 
                    
                { // CALL | (prelude.ceu : lin 296 : col 9) | string?(it)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 50) /* global string? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local it */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 296, col 9) : string?(it)");
            
        
                } // CALL | (prelude.ceu : lin 296 : col 9) | string?(it)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 296 : col 21) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(   atoi((ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Dyn->Vector.buf))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 296 : col 21) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 298 : col 36) | if true nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 301 : col 29) | if true nil
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 297 : col 14) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 297 : col 14) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 299 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 299 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 299 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 298 : col 36) | if true nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 297 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 296 : col 36) | (val it :char
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 299 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 292 : col 20) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 301 : col 1) | (func (v) do 
                    int ceu_f_to_dash_tag (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 301 : col 17) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 307 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 308 : col 25) | (val ceu_11602 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_11602 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 308 : col 25) | (val ceu_11602 )
        
            
                // DCL | (prelude.ceu : lin 304 : col 29) | (val it :tag = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11602 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 304 : col 29) | (val it :tag = )
        
            
                { // IF | (prelude.ceu : lin 304 : col 29) | if is'(it,:tag)
                    
                { // CALL | (prelude.ceu : lin 303 : col 16) | is'(it,:tag)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 303, col 16) : is'(it,:tag)");
            
        
                } // CALL | (prelude.ceu : lin 303 : col 16) | is'(it,:tag)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 303 : col 22) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 303 : col 22) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 305 : col 36) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 307 : col 29) | (val it = ceu_1
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11602 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 307 : col 29) | (val it = ceu_1)
        
            
                { // IF | (prelude.ceu : lin 304 : col 29) | if string?(it) 
                    
                { // CALL | (prelude.ceu : lin 304 : col 9) | string?(it)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 50) /* global string? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 304, col 9) : string?(it)");
            
        
                } // CALL | (prelude.ceu : lin 304 : col 9) | string?(it)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 304 : col 21) | to-tag-string
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 304 : col 24) | to-tag-string(v
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 10) /* global to-tag-string */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 304, col 24) : to-tag-string(v)");
            
        
                } // CALL | (prelude.ceu : lin 304 : col 24) | to-tag-string(v
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 304 : col 21) | to-tag-string
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 306 : col 36) | if true nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 309 : col 29) | if true nil
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 305 : col 14) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 305 : col 14) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 307 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 307 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 307 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 306 : col 36) | if true nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 305 : col 36) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 307 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 301 : col 17) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 309 : col 1) | (func (v) do 
                    int ceu_f_to_dash_pointer (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 309 : col 21) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 318 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 319 : col 25) | (val ceu_11955 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 319 : col 25) | (val ceu_11955 )
        
            
                // DCL | (prelude.ceu : lin 312 : col 29) | (val it :pointe
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 312 : col 29) | (val it :pointe)
        
            
                { // IF | (prelude.ceu : lin 312 : col 29) | if is'(it,:poin
                    
                { // CALL | (prelude.ceu : lin 311 : col 16) | is'(it,:pointer
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_pointer} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 311, col 16) : is'(it,:pointer)");
            
        
                } // CALL | (prelude.ceu : lin 311 : col 16) | is'(it,:pointer
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 311 : col 26) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 311 : col 26) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 313 : col 36) | (val it :tag 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 315 : col 29) | (val it :tag = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 315 : col 29) | (val it :tag = )
        
            
                { // IF | (prelude.ceu : lin 313 : col 29) | if is'(it,:tag)
                    
                { // CALL | (prelude.ceu : lin 312 : col 16) | is'(it,:tag)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 312, col 16) : is'(it,:tag)");
            
        
                } // CALL | (prelude.ceu : lin 312 : col 16) | is'(it,:tag)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 312 : col 22) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(   ceu_to_dash_string_dash_tag((ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Tag))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 312 : col 22) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 314 : col 36) | (val it :tupl
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 316 : col 29) | (val it :tuple 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 316 : col 29) | (val it :tuple )
        
            
                { // IF | (prelude.ceu : lin 314 : col 29) | if is'(it,:tupl
                    
                { // CALL | (prelude.ceu : lin 313 : col 16) | is'(it,:tuple)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tuple} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 313, col 16) : is'(it,:tuple)");
            
        
                } // CALL | (prelude.ceu : lin 313 : col 16) | is'(it,:tuple)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 313 : col 24) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(   (ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Dyn->Tuple.buf)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 313 : col 24) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 315 : col 36) | (val it :vect
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 317 : col 29) | (val it :vector
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 317 : col 29) | (val it :vector)
        
            
                { // IF | (prelude.ceu : lin 315 : col 29) | if is'(it,:vect
                    
                { // CALL | (prelude.ceu : lin 314 : col 16) | is'(it,:vector)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 314, col 16) : is'(it,:vector)");
            
        
                } // CALL | (prelude.ceu : lin 314 : col 16) | is'(it,:vector)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 314 : col 25) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(   (ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Dyn->Vector.buf)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 314 : col 25) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 316 : col 36) | (val it :dict
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 318 : col 29) | (val it :dict =
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_11955 */));

                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 318 : col 29) | (val it :dict =)
        
            
                { // IF | (prelude.ceu : lin 316 : col 29) | if is'(it,:dict
                    
                { // CALL | (prelude.ceu : lin 315 : col 16) | is'(it,:dict)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 5 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_dict} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 315, col 16) : is'(it,:dict)");
            
        
                } // CALL | (prelude.ceu : lin 315 : col 16) | is'(it,:dict)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 315 : col 23) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(   (ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Dyn->Dict.buf)} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 315 : col 23) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 317 : col 36) | if true nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 320 : col 29) | if true nil
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 316 : col 14) | nil
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 316 : col 14) | nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 318 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 318 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 318 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 317 : col 36) | if true nil
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 316 : col 36) | (val it :dict
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 315 : col 36) | (val it :vect
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 314 : col 36) | (val it :tupl
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 313 : col 36) | (val it :tag 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 318 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 309 : col 21) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 324 : col 1) | (func (v) <
                    int ceu_f_type_dash_static_question_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 324 : col 23) | <(to-numb
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 325 : col 18) | <(to-number
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 30) /* global {{<}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 325 : col 5) | to-number(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 53) /* global to-number */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 325, col 5) : to-number(v)");
            
        
                } // CALL | (prelude.ceu : lin 325 : col 5) | to-number(v)
                
                    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  CEU_VALUE_DYNAMIC)} }));
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 325, col 18) : {{<}}(to-number(v),```:number   CEU_VALUE_DYN...)");
            
        
                } // CALL | (prelude.ceu : lin 325 : col 18) | <(to-number
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 324 : col 23) | <(to-numb
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 327 : col 1) | (func (v) >
                    int ceu_f_type_dash_dynamic_question_ (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 327 : col 24) | >(to-numb
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 328 : col 18) | >(to-number
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 328 : col 5) | to-number(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 53) /* global to-number */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 328, col 5) : to-number(v)");
            
        
                } // CALL | (prelude.ceu : lin 328 : col 5) | to-number(v)
                
                    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  CEU_VALUE_DYNAMIC)} }));
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 328, col 18) : {{>}}(to-number(v),```:number   CEU_VALUE_DYN...)");
            
        
                } // CALL | (prelude.ceu : lin 328 : col 18) | >(to-number
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 327 : col 24) | >(to-numb
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 333 : col 1) | (func :rec (v) 
                    int ceu_f_copy (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 333 : col 20) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 349 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 350 : col 25) | (val ceu_12662 
                
                    
                { // CALL | (prelude.ceu : lin 334 : col 9) | type(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 334, col 9) : type(v)");
            
        
                } // CALL | (prelude.ceu : lin 334 : col 9) | type(v)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_12662 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 350 : col 25) | (val ceu_12662 )
        
            
                // DCL | (prelude.ceu : lin 336 : col 29) | (val it = ceu_1
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_12662 */));

                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 336 : col 29) | (val it = ceu_1)
        
            
                { // IF | (prelude.ceu : lin 335 : col 29) | if type-static?
                    
                { // CALL | (prelude.ceu : lin 335 : col 9) | type-static?(it
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 56) /* global type-static? */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local it */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 335, col 9) : type-static?(it)");
            
        
                } // CALL | (prelude.ceu : lin 335 : col 9) | type-static?(it
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 335 : col 26) | v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 335 : col 26) | v
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 338 : col 36) | (val it :tupl
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 340 : col 29) | (val it :tuple 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_12662 */));

                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 340 : col 29) | (val it :tuple )
        
            
                { // IF | (prelude.ceu : lin 339 : col 29) | if is'(it,:tupl
                    
                { // CALL | (prelude.ceu : lin 338 : col 16) | is'(it,:tuple)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tuple} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 338, col 16) : is'(it,:tuple)");
            
        
                } // CALL | (prelude.ceu : lin 338 : col 16) | is'(it,:tuple)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 338 : col 24) | (val ret = tu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 339 : col 13) | (val ret = tupl
                
                    
                { // CALL | (prelude.ceu : lin 339 : col 23) | tuple(#(v))
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 11) /* global tuple */));

                    
                        
                { // CALL | (prelude.ceu : lin 339 : col 29) | #(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 339, col 29) : {{#}}(v)");
            
        
                } // CALL | (prelude.ceu : lin 339 : col 29) | #(v)
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 339, col 23) : tuple({{#}}(v))");
            
        
                } // CALL | (prelude.ceu : lin 339 : col 23) | tuple(#(v))
                
                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local ret */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 339 : col 13) | (val ret = tupl)
        
            
                { // BLOCK | (prelude.ceu : lin 343 : col 29) | do (val ceu_s
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 3 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 344 : col 33) | (val ceu_ste_12
                
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local ceu_ste_12545 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 344 : col 33) | (val ceu_ste_12)
        
            
                // DCL | (prelude.ceu : lin 345 : col 33) | (var i = +(
                
                    
                { // CALL | (prelude.ceu : lin 340 : col 26) | +(0,0)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 340, col 26) : {{+}}(0,0)");
            
        
                } // CALL | (prelude.ceu : lin 340 : col 26) | +(0,0)
                
                    ceux_copy(X->S, (X->base + 0 + 4 + 1) /* local i */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 345 : col 33) | (var i = +()
        
            
                // DCL | (prelude.ceu : lin 343 : col 33) | (val ceu_lim_12
                
                    
                { // CALL | (prelude.ceu : lin 340 : col 29) | #(v)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 340, col 29) : {{#}}(v)");
            
        
                } // CALL | (prelude.ceu : lin 340 : col 29) | #(v)
                
                    ceux_copy(X->S, (X->base + 0 + 4 + 2) /* local ceu_lim_12545 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 343 : col 33) | (val ceu_lim_12)
        
            
                // LOOP | (prelude.ceu : lin 341 : col 33) | loop (break(f
                CEU_LOOP_START_13077:
                    
                { // BLOCK | (prelude.ceu : lin 341 : col 33) | (break(false)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                         // BREAK | (prelude.ceu : lin 342 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 342 : col 56) | >=(i,ceu_li
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 31) /* global {{>=}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 1) /* local i */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 2) /* local ceu_lim_12545 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 342, col 56) : {{>=}}(i,ceu_lim_12545)");
            
        
                } // CALL | (prelude.ceu : lin 342 : col 56) | >=(i,ceu_li
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 342 : col 43) | false)
        
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_13077;
                    }
                }
            
                { // SET | (prelude.ceu : lin 341 : col 17) | (set ret[i] = c
                    
                { // CALL | (prelude.ceu : lin 341 : col 30) | copy(v[i])
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 58) /* global copy */));

                    
                        
                { // INDEX | (prelude.ceu : lin 341 : col 35) | v[i]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 1) /* local i */));

                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 341, col 35) : v[i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 341, col 35) : v[i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 341, col 30) : copy(v[i])");
            
        
                } // CALL | (prelude.ceu : lin 341 : col 30) | copy(v[i])
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 341 : col 21) | ret[i]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 1) /* local i */));

                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ret */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 341, col 21) : ret[i]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 341, col 21) : ret[i]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                { // SET | (prelude.ceu : lin 343 : col 37) | (set i = +(
                    
                { // CALL | (prelude.ceu : lin 343 : col 47) | +(i,ceu_ste
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 1) /* local i */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local ceu_ste_12545 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 343, col 47) : {{+}}(i,ceu_ste_12545)");
            
        
                } // CALL | (prelude.ceu : lin 343 : col 47) | +(i,ceu_ste
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                        // ACC - SET | (prelude.ceu : lin 343 : col 41) | i
                        ceux_repl(X->S, (X->base + 0 + 4 + 1) /* local i */, ceux_peek(X->S,XX(-1)));
                      // dst should not pop src
                }
            
                        
                            CEU_LOOP_STOP_13077:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 341 : col 33) | (break(false)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  0;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_13077;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 343 : col 29) | do (val ceu_s
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+4, 3, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ret */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 338 : col 24) | (val ret = tu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 345 : col 36) | if true err
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 348 : col 29) | if true error
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 345 : col 14) | error(:TODO)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 346 : col 13) | error(:TODO)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 1) /* global error */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_TODO} });
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 346, col 13) : error(:TODO)");
            
        
                } // CALL | (prelude.ceu : lin 346 : col 13) | error(:TODO)
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 345 : col 14) | error(:TODO)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 348 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (prelude.ceu : lin 348 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 348 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 345 : col 36) | if true err
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 338 : col 36) | (val it :tupl
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 349 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 333 : col 20) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 351 : col 1) | (func (col,tp) 
                    int ceu_f_to_dash_vector (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 351 : col 26) | (val ret = #[
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 352 : col 5) | (val ret = #[])
                
                    
                { // VECTOR | (prelude.ceu : lin 352 : col 15) | #[]
                    ceux_push(X->S, 1, ceu_create_vector());
                    
                }
            
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ret */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 352 : col 5) | (val ret = #[]))
        
            
                { // BLOCK | (prelude.ceu : lin 356 : col 29) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 357 : col 33) | (val ceu_13249 
                
                    
                { // CALL | (prelude.ceu : lin 357 : col 49) | to-iter(to-iter
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        
                { // CALL | (prelude.ceu : lin 353 : col 15) | to-iter(col,tp)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg col */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg tp */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 353, col 15) : to-iter(col,tp)");
            
        
                } // CALL | (prelude.ceu : lin 353 : col 15) | to-iter(col,tp)
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 357, col 49) : to-iter(to-iter(col,tp))");
            
        
                } // CALL | (prelude.ceu : lin 357 : col 49) | to-iter(to-iter
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_13249 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 357 : col 33) | (val ceu_13249 )
        
            
                // LOOP | (prelude.ceu : lin 354 : col 33) | loop (val v =
                CEU_LOOP_START_13472:
                    
                { // BLOCK | (prelude.ceu : lin 354 : col 33) | (val v = ceu_
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 355 : col 37) | (val v = ceu_13
                
                    
                { // CALL | (prelude.ceu : lin 353 : col 14) | ceu_13249[0](ce
                    
                { // INDEX | (prelude.ceu : lin 353 : col 14) | ceu_13249[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_13249 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 353, col 14) : ceu_13249[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 353, col 14) : ceu_13249[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_13249 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 353, col 14) : ceu_13249[0](ceu_13249)");
            
        
                } // CALL | (prelude.ceu : lin 353 : col 14) | ceu_13249[0](ce
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local v */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 355 : col 37) | (val v = ceu_13)
        
             // BREAK | (prelude.ceu : lin 354 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 354 : col 56) | ==(v,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local v */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 354, col 56) : {{==}}(v,nil)");
            
        
                } // CALL | (prelude.ceu : lin 354 : col 56) | ==(v,nil)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 354 : col 43) | false)
        
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_13472;
                    }
                }
            
                { // BLOCK | (prelude.ceu : lin 355 : col 29) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 354 : col 24) | (val ceu_13159 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ret */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local ceu_13159 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 354 : col 24) | (val ceu_13159 )
        
            
                { // SET | (prelude.ceu : lin 355 : col 33) | (set ceu_13159[
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local v */));
  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 355 : col 37) | ceu_13159[#
                    // IDX
                    
                { // CALL | (prelude.ceu : lin 355 : col 47) | #(ceu_13159
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ceu_13159 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 355, col 47) : {{#}}(ceu_13159)");
            
        
                } // CALL | (prelude.ceu : lin 355 : col 47) | #(ceu_13159
                
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ceu_13159 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 355, col 37) : ceu_13159[{{#}}(ceu_13159)]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 355, col 37) : ceu_13159[{{#}}(ceu_13159)]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 355 : col 29) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                            CEU_LOOP_STOP_13472:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 354 : col 33) | (val v = ceu_
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  0;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_13472;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 356 : col 29) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ret */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 351 : col 26) | (val ret = #[
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 379 : col 1) | (func (col) (
                    int ceu_f_to_dash_set (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 379 : col 19) | (val ret = @[
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 380 : col 5) | (val ret = @[])
                
                    
                { // DICT | (prelude.ceu : lin 380 : col 15) | @[]
                    ceux_push(X->S, 1, ceu_create_dict());
                    
                }
            
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ret */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 380 : col 5) | (val ret = @[]))
        
            
                { // BLOCK | (prelude.ceu : lin 384 : col 29) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 385 : col 33) | (val ceu_13521 
                
                    
                { // CALL | (prelude.ceu : lin 385 : col 49) | to-iter(to-iter
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        
                { // CALL | (prelude.ceu : lin 381 : col 13) | to-iter(col)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg col */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 381, col 13) : to-iter(col)");
            
        
                } // CALL | (prelude.ceu : lin 381 : col 13) | to-iter(col)
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 385, col 49) : to-iter(to-iter(col))");
            
        
                } // CALL | (prelude.ceu : lin 385 : col 49) | to-iter(to-iter
                
                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_13521 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 385 : col 33) | (val ceu_13521 )
        
            
                // LOOP | (prelude.ceu : lin 382 : col 33) | loop (val it 
                CEU_LOOP_START_13700:
                    
                { // BLOCK | (prelude.ceu : lin 382 : col 33) | (val it = ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 383 : col 37) | (val it = ceu_1
                
                    
                { // CALL | (prelude.ceu : lin 381 : col 10) | ceu_13521[0](ce
                    
                { // INDEX | (prelude.ceu : lin 381 : col 10) | ceu_13521[0]
                    // IDX
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_13521 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 381, col 10) : ceu_13521[0]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 381, col 10) : ceu_13521[0]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_13521 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 381, col 10) : ceu_13521[0](ceu_13521)");
            
        
                } // CALL | (prelude.ceu : lin 381 : col 10) | ceu_13521[0](ce
                
                    ceux_copy(X->S, (X->base + 0 + 2 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 383 : col 37) | (val it = ceu_1)
        
             // BREAK | (prelude.ceu : lin 382 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 382 : col 57) | ==(it,nil)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 382, col 57) : {{==}}(it,nil)");
            
        
                } // CALL | (prelude.ceu : lin 382 : col 57) | ==(it,nil)
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 382 : col 43) | false)
        
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_13700;
                    }
                }
            
                { // SET | (prelude.ceu : lin 382 : col 9) | (set ret[it] = 
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 382 : col 13) | ret[it]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 2 + 0) /* local it */));

                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ret */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 382, col 13) : ret[it]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 382, col 13) : ret[it]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                        
                            CEU_LOOP_STOP_13700:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 382 : col 33) | (val it = ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  0;
                        ceux_block_leave(X->S, X->base+2, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_13700;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 384 : col 29) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+1, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ret */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 379 : col 19) | (val ret = @[
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 391 : col 1) | (func (v1,v2) 
                    int ceu_f_less_plus_plus (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 391 : col 23) | assert(==
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 392 : col 5) | assert(==(t
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 33) /* global assert */));

                    
                        
                { // CALL | (prelude.ceu : lin 392 : col 21) | ==(type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 392 : col 12) | type(v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 392, col 12) : type(v1)");
            
        
                } // CALL | (prelude.ceu : lin 392 : col 12) | type(v1)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 392, col 21) : {{==}}(type(v1),:vector)");
            
        
                } // CALL | (prelude.ceu : lin 392 : col 21) | ==(type(v1)
                
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 392, col 5) : assert({{==}}(type(v1),:vector))");
            
        
                } // CALL | (prelude.ceu : lin 392 : col 5) | assert(==(t
                
                { // CALL | (prelude.ceu : lin 393 : col 5) | assert(==(t
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 33) /* global assert */));

                    
                        
                { // CALL | (prelude.ceu : lin 393 : col 21) | ==(type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 393 : col 12) | type(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 12) /* global type */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 393, col 12) : type(v2)");
            
        
                } // CALL | (prelude.ceu : lin 393 : col 12) | type(v2)
                
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_vector} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 393, col 21) : {{==}}(type(v2),:vector)");
            
        
                } // CALL | (prelude.ceu : lin 393 : col 21) | ==(type(v2)
                
                                        
                    

                    ceux_call(X, 1, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 393, col 5) : assert({{==}}(type(v2),:vector))");
            
        
                } // CALL | (prelude.ceu : lin 393 : col 5) | assert(==(t
                
                { // BLOCK | (prelude.ceu : lin 394 : col 5) | do do (val 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (prelude.ceu : lin 398 : col 29) | do (val ceu_s
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 3 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 399 : col 33) | (val ceu_ste_13
                
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local ceu_ste_13877 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 399 : col 33) | (val ceu_ste_13)
        
            
                // DCL | (prelude.ceu : lin 400 : col 33) | (var i = +(
                
                    
                { // CALL | (prelude.ceu : lin 395 : col 22) | +(0,0)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 395, col 22) : {{+}}(0,0)");
            
        
                } // CALL | (prelude.ceu : lin 395 : col 22) | +(0,0)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local i */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 400 : col 33) | (var i = +()
        
            
                // DCL | (prelude.ceu : lin 398 : col 33) | (val ceu_lim_13
                
                    
                { // CALL | (prelude.ceu : lin 395 : col 25) | #(v2)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 395, col 25) : {{#}}(v2)");
            
        
                } // CALL | (prelude.ceu : lin 395 : col 25) | #(v2)
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 2) /* local ceu_lim_13877 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 398 : col 33) | (val ceu_lim_13)
        
            
                // LOOP | (prelude.ceu : lin 396 : col 33) | loop (break(f
                CEU_LOOP_START_14170:
                    
                { // BLOCK | (prelude.ceu : lin 396 : col 33) | (break(false)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                         // BREAK | (prelude.ceu : lin 397 : col 37) | (break(false) i
                
                { // CALL | (prelude.ceu : lin 397 : col 56) | >=(i,ceu_li
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 31) /* global {{>=}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local i */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local ceu_lim_13877 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 397, col 56) : {{>=}}(i,ceu_lim_13877)");
            
        
                } // CALL | (prelude.ceu : lin 397 : col 56) | >=(i,ceu_li
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            ceux_pop(X->S, 1);        // (2)
                            ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 397 : col 43) | false)
        
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_14170;
                    }
                }
            
                { // BLOCK | (prelude.ceu : lin 397 : col 29) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (prelude.ceu : lin 396 : col 27) | (val ceu_13777 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local ceu_13777 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 396 : col 27) | (val ceu_13777 )
        
            
                { // SET | (prelude.ceu : lin 397 : col 33) | (set ceu_13777[
                    
                { // INDEX | (prelude.ceu : lin 396 : col 25) | v2[i]
                    // IDX
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local i */));

                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 396, col 25) : v2[i]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "prelude.ceu : (lin 396, col 25) : v2[i]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                { // INDEX | (prelude.ceu : lin 397 : col 37) | ceu_13777[#
                    // IDX
                    
                { // CALL | (prelude.ceu : lin 397 : col 47) | #(ceu_13777
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 13) /* global {{#}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ceu_13777 */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 397, col 47) : {{#}}(ceu_13777)");
            
        
                } // CALL | (prelude.ceu : lin 397 : col 47) | #(ceu_13777
                
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local ceu_13777 */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "prelude.ceu : (lin 397, col 37) : ceu_13777[{{#}}(ceu_13777)]"
                    );
                
                        {
                            CEU_Value v = ceu_col_set(ceux_peek(X->S,XX(-1)), ceux_peek(X->S,XX(-2)), ceux_peek(X->S,XX(-3)));
                            CEU_ERROR_CHK_VAL(continue, v, "prelude.ceu : (lin 397, col 37) : ceu_13777[{{#}}(ceu_13777)]");
                            ceux_drop(X->S, 2);    // keep src
                        }
                        
                }
                  // dst should not pop src
                }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 397 : col 29) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                { // SET | (prelude.ceu : lin 399 : col 37) | (set i = +(
                    
                { // CALL | (prelude.ceu : lin 399 : col 47) | +(i,ceu_ste
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local i */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local ceu_ste_13877 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 399, col 47) : {{+}}(i,ceu_ste_13877)");
            
        
                } // CALL | (prelude.ceu : lin 399 : col 47) | +(i,ceu_ste
                  // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                        // ACC - SET | (prelude.ceu : lin 399 : col 41) | i
                        ceux_repl(X->S, (X->base + 0 + 0 + 1) /* local i */, ceux_peek(X->S,XX(-1)));
                      // dst should not pop src
                }
            
                        
                            CEU_LOOP_STOP_14170:
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 396 : col 33) | (break(false)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  0;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_14170;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 398 : col 29) | do (val ceu_s
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 3, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 394 : col 5) | do do (val 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 391 : col 23) | assert(==
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 402 : col 1) | (func (v1,v2) 
                    int ceu_f_plus_plus (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 402 : col 22) | <++(<++
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 403 : col 18) | <++(<++
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 61) /* global {{<++}} */));

                    
                        
                { // CALL | (prelude.ceu : lin 403 : col 10) | <++(#[],v1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 61) /* global {{<++}} */));

                    
                        
                { // VECTOR | (prelude.ceu : lin 403 : col 6) | #[]
                    ceux_push(X->S, 1, ceu_create_vector());
                    
                }
            
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v1 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 403, col 10) : {{<++}}(#[],v1)");
            
        
                } // CALL | (prelude.ceu : lin 403 : col 10) | <++(#[],v1)
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg v2 */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 403, col 18) : {{<++}}({{<++}}(#[],v1),v2)");
            
        
                } // CALL | (prelude.ceu : lin 403 : col 18) | <++(<++
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 402 : col 22) | <++(<++
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 409 : col 5) | (func (v) f(g
                    int ceu_f_14315 (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 409 : col 14) | f(g(v))
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+2, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (prelude.ceu : lin 410 : col 9) | f(g(v))
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0) /* upval f */));

                    
                        
                { // CALL | (prelude.ceu : lin 410 : col 11) | g(v)
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 1) /* upval g */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg v */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 410, col 11) : g(v)");
            
        
                } // CALL | (prelude.ceu : lin 410 : col 11) | g(v)
                
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 410, col 9) : f(g(v))");
            
        
                } // CALL | (prelude.ceu : lin 410 : col 9) | f(g(v))
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 409 : col 14) | f(g(v))
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+2, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 408 : col 1) | (func (f,g) (
                    int ceu_f_less_bar_less (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 408 : col 20) | (func (v) f
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                         // CREATE | (prelude.ceu : lin 409 : col 5) | (func (v) f(g
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_14315,
                        1,  // TODO: remove assert
                        0,
                        2
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 2
                    
                    {
                        CEU_Value up = ceux_peek(X->S, ceux_arg(X, 0) /* arg f */);
                        ceu_gc_inc_val(up);
                        clo.Dyn->Clo.upvs.buf[0] = up;
                    }
                    

                    {
                        CEU_Value up = ceux_peek(X->S, ceux_arg(X, 1) /* arg g */);
                        ceu_gc_inc_val(up);
                        clo.Dyn->Clo.upvs.buf[1] = up;
                    }
                    
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 408 : col 20) | (func (v) f
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 418 : col 1) | (func (n) if 
                    int ceu_f_random_dash_seed (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 418 : col 22) | if n ```sra
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (prelude.ceu : lin 419 : col 5) | if n ```srand
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg n */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (prelude.ceu : lin 419 : col 10) | ```srand($n.N
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        srand((ceux_peek(X->S,ceux_arg(X, 0) /* arg n */)).Number);

                        
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 420, col 9) : ```srand($n.Number);```");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 419 : col 10) | ```srand($n.N
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (prelude.ceu : lin 421 : col 12) | ```srand(time
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        srand(time(NULL));

                        
            CEU_ERROR_CHK_STK(continue, "prelude.ceu : (lin 422, col 9) : ```srand(time(NULL));```");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 421 : col 12) | ```srand(time
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 418 : col 22) | if n ```sra
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 426 : col 1) | (func () ```:
                    int ceu_f_random_dash_next (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 426 : col 21) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  rand())} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 426 : col 21) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 430 : col 1) | (func (v) ```
                    int ceu_f_math_dash_sin (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 430 : col 19) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  sin((ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 430 : col 19) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (prelude.ceu : lin 434 : col 1) | (func (v) ```
                    int ceu_f_math_dash_cos (CEUX* X) {
                        
                        do {
                { // BLOCK | (prelude.ceu : lin 434 : col 19) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  cos((ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (prelude.ceu : lin 434 : col 19) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 438 : col 1) | (func (v) ```
                    int ceu_f_math_dash_floor (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 438 : col 21) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  floor((ceux_peek(X->S,ceux_arg(X, 0) /* arg v */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 438 : col 21) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
#include <iup.h>
int iup_action_cb (Ihandle* h) {
    CEU_Value evt = ceu_create_tuple(1);
    CEU_Value tag = { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iup_Action} };
    ceu_tags_set(evt, tag, 1);
    ceu_tuple_set(&evt.Dyn->Tuple, 0, (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=h} });
    ceux_push(CEU_GLOBAL_X->S, 1, evt);
    int ret = ceu_broadcast_global();
    if (ret != 0) {
        IupExitLoop();
        return IUP_CLOSE;   // TODO
    }
    ceux_pop(CEU_GLOBAL_X->S, 1);
    return IUP_DEFAULT;
}

int iup_value_cb (Ihandle* h) {
    CEU_Value evt = ceu_create_tuple(1);
    CEU_Value tag = { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iup_Value} };
    ceu_tags_set(evt, tag, 1);
    ceu_tuple_set(&evt.Dyn->Tuple, 0, (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=h} });
    ceux_push(CEU_GLOBAL_X->S, 1, evt);
    int ret = ceu_broadcast_global();
    if (ret != 0) {
        IupExitLoop();
        return IUP_CLOSE;   // TODO
    }
    ceux_pop(CEU_GLOBAL_X->S, 1);
    return IUP_DEFAULT;
}

int iup_list_action_cb (Ihandle* h, char *text, int item, int state) {
    CEU_Value evt = ceu_create_tuple(3);
    CEU_Value tag = { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iup_List_Action} };
    ceu_tags_set(evt, tag, 1);
    ceu_tuple_set(&evt.Dyn->Tuple, 0, (CEU_Value) { CEU_VALUE_POINTER, {.Pointer=h} });
    ceu_tuple_set(&evt.Dyn->Tuple, 1, (CEU_Value) { CEU_VALUE_NUMBER,  {.Number=item} });
    ceu_tuple_set(&evt.Dyn->Tuple, 2, (CEU_Value) { CEU_VALUE_BOOL,    {.Bool=state} });
    ceux_push(CEU_GLOBAL_X->S, 1, evt);
    int ret = ceu_broadcast_global();
    if (ret != 0) {
        IupExitLoop();
        return IUP_CLOSE;   // TODO
    }
    ceux_pop(CEU_GLOBAL_X->S, 1);
    return IUP_DEFAULT;
}

                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 61 : col 1) | (func () ```I
                    int ceu_f_iup_dash_loop (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 61 : col 18) | ```IupMainLoo
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        IupMainLoop();

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 62, col 5) : ```IupMainLoop();```");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 61 : col 18) | ```IupMainLoo
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 67 : col 1) | (func (h,name) 
                    int ceu_f_iup_dash_get_dash_pointer (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 67 : col 32) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 68 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 68 : col 13) | to-pointer(name
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg name */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 68, col 13) : to-pointer(name)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 68 : col 13) | to-pointer(name
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 68 : col 5) | (val s = to-poi)
        
            ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupGetAttribute((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 67 : col 32) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 71 : col 1) | (func (h,name) 
                    int ceu_f_iup_dash_get_dash_bool (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 71 : col 29) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 72 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 72 : col 13) | to-pointer(name
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg name */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 72, col 13) : to-pointer(name)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 72 : col 13) | to-pointer(name
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 72 : col 5) | (val s = to-poi)
        
            ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_BOOL, {.Bool=(  IupGetInt((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 71 : col 29) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 75 : col 1) | (func (h,name) 
                    int ceu_f_iup_dash_get_dash_number (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 75 : col 31) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 76 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 76 : col 13) | to-pointer(name
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg name */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 76, col 13) : to-pointer(name)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 76 : col 13) | to-pointer(name
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 76 : col 5) | (val s = to-poi)
        
            ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  IupGetInt((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 75 : col 31) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 80 : col 1) | (func (h,name,v
                    int ceu_f_iup_dash_set (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 80 : col 30) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 81 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 81 : col 13) | to-pointer(name
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 1) /* arg name */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 81, col 13) : to-pointer(name)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 81 : col 13) | to-pointer(name
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 81 : col 5) | (val s = to-poi)
        
            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 96 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+1, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 97 : col 25) | (val ceu_14951 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 2) /* arg value */));

                    ceux_copy(X->S, (X->base + 0 + 1 + 0) /* local ceu_14951 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 97 : col 25) | (val ceu_14951 )
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 29) | (val it :bool =
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_14951 */));

                    ceux_copy(X->S, (X->base + 0 + 1 + 1) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 84 : col 29) | (val it :bool =)
        
            
                { // IF | (@/iup/iup-ceu/iup.ceu : lin 84 : col 29) | if is'(it,:bool
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 83 : col 16) | is'(it,:bool)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 1) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_bool} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 83, col 16) : is'(it,:bool)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 83 : col 16) | is'(it,:bool)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 83 : col 23) | ```IupSetInt(
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        IupSetInt((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1, (ceux_peek(X->S,ceux_arg(X, 2) /* arg value */)).Bool);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 83, col 21) : ```IupSetInt($h.Pointer, $s.Pointer+1, $value...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 83 : col 23) | ```IupSetInt(
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 85 : col 36) | (val it :numb
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+3, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 87 : col 29) | (val it :number
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_14951 */));

                    ceux_copy(X->S, (X->base + 0 + 3 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 87 : col 29) | (val it :number)
        
            
                { // IF | (@/iup/iup-ceu/iup.ceu : lin 85 : col 29) | if is'(it,:numb
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 16) | is'(it,:number)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 3 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_number} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 84, col 16) : is'(it,:number)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 16) | is'(it,:number)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 84 : col 25) | if ==(mat
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (@/iup/iup-ceu/iup.ceu : lin 84 : col 21) | if ==(math-
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 42) | ==(math-flo
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 24) | math-floor(valu
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 68) /* global math-floor */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 2) /* arg value */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 84, col 24) : math-floor(value)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 24) | math-floor(valu
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 2) /* arg value */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 84, col 42) : {{==}}(math-floor(value),value)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 84 : col 42) | ==(math-flo
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 85 : col 25) | ```IupSetInt(
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        IupSetInt((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1, (ceux_peek(X->S,ceux_arg(X, 2) /* arg value */)).Number);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 85, col 28) : ```IupSetInt($h.Pointer, $s.Pointer+1, $value...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 85 : col 25) | ```IupSetInt(
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 86 : col 25) | ```IupSetFloa
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        IupSetFloat((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1, (ceux_peek(X->S,ceux_arg(X, 2) /* arg value */)).Number);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 86, col 28) : ```IupSetFloat($h.Pointer, $s.Pointer+1, $val...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 86 : col 25) | ```IupSetFloa
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 84 : col 25) | if ==(mat
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 89 : col 36) | (val it :tag 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 91 : col 29) | (val it :tag = 
                
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 1 + 0) /* local ceu_14951 */));

                    ceux_copy(X->S, (X->base + 0 + 4 + 0) /* local it */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 91 : col 29) | (val it :tag = )
        
            
                { // IF | (@/iup/iup-ceu/iup.ceu : lin 88 : col 29) | if is'(it,:tag)
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 87 : col 16) | is'(it,:tag)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_tag} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 87, col 16) : is'(it,:tag)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 87 : col 16) | is'(it,:tag)
                
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 87 : col 22) | do (val t =
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 88 : col 13) | do (val t = t
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 89 : col 17) | (val t = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 89 : col 25) | to-pointer(valu
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 2) /* arg value */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 89, col 25) : to-pointer(value)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 89 : col 25) | to-pointer(valu
                
                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local t */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 89 : col 17) | (val t = to-poi)
        
            IupSetAttribute((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1, (ceux_peek(X->S,(X->base + 0 + 5 + 0) /* local t */)).Pointer+1);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 87, col 21) : ```IupSetAttribute($h.Pointer, $s.Pointer+1, ...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 88 : col 13) | do (val t = t
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 87 : col 22) | do (val t =
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 90 : col 36) | if true do 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // IF | (@/iup/iup-ceu/iup.ceu : lin 93 : col 29) | if true do 
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 91 : col 14) | do (val v =
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 92 : col 13) | do (val v = t
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 93 : col 17) | (val v = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 93 : col 25) | to-pointer(valu
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 2) /* arg value */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 93, col 25) : to-pointer(value)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 93 : col 25) | to-pointer(valu
                
                    ceux_copy(X->S, (X->base + 0 + 5 + 0) /* local v */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 93 : col 17) | (val v = to-poi)
        
            IupSetStrAttribute((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer+1, (ceux_peek(X->S,(X->base + 0 + 5 + 0) /* local v */)).Pointer);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 91, col 21) : ```IupSetStrAttribute($h.Pointer, $s.Pointer+...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 92 : col 13) | do (val v = t
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 91 : col 14) | do (val v =
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } else {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 94 : col 36) | (do nil)
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+5, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // PASS | (@/iup/iup-ceu/iup.ceu : lin 94 : col 36) | (do nil)
ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 94 : col 36) | (do nil)
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 90 : col 36) | if true do 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+5, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 89 : col 36) | (val it :tag 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+4, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 85 : col 36) | (val it :numb
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+3, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 96 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+1, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 80 : col 30) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 100 : col 1) | (func (label) 
                    int ceu_f_iup_dash_button (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 100 : col 25) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 101 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 101 : col 13) | to-pointer(labe
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg label */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 101, col 13) : to-pointer(label)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 101 : col 13) | to-pointer(labe
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 101 : col 5) | (val s = to-poi)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 102 : col 5) | (val h = ```:po
                
                    ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupButton((ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer, NULL))} }));
                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local h */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 102 : col 5) | (val h = ```:po)
        
            IupSetCallback((ceux_peek(X->S,(X->base + 0 + 0 + 1) /* local h */)).Pointer, "ACTION", iup_action_cb);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 103, col 5) : ```IupSetCallback($h.Pointer, 'ACTION', iup_a...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 103 : col 5) | ```IupSetCallba)
        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local h */));

                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 100 : col 25) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 107 : col 1) | (func (label) 
                    int ceu_f_iup_dash_label (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 107 : col 24) | (val s = to-p
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 108 : col 5) | (val s = to-poi
                
                    
                { // CALL | (@/iup/iup-ceu/iup.ceu : lin 108 : col 13) | to-pointer(labe
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, ceux_arg(X, 0) /* arg label */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 108, col 13) : to-pointer(label)");
            
        
                } // CALL | (@/iup/iup-ceu/iup.ceu : lin 108 : col 13) | to-pointer(labe
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local s */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 108 : col 5) | (val s = to-poi)
        
            ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupLabel((ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local s */)).Pointer))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 107 : col 24) | (val s = to-p
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 112 : col 1) | (func () (val
                    int ceu_f_iup_dash_list (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 112 : col 18) | (val h = ```:
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 113 : col 5) | (val h = ```:po
                
                    ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupList(NULL))} }));
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local h */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 113 : col 5) | (val h = ```:po)
        
            IupSetCallback((ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local h */)).Pointer, "ACTION", (Icallback) iup_list_action_cb);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 114, col 5) : ```IupSetCallback($h.Pointer, 'ACTION', (Ical...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 114 : col 5) | ```IupSetCallba)
        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local h */));

                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 112 : col 18) | (val h = ```:
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 118 : col 1) | (func () (val
                    int ceu_f_iup_dash_text (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 118 : col 18) | (val h = ```:
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 119 : col 5) | (val h = ```:po
                
                    ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupText(NULL))} }));
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local h */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 119 : col 5) | (val h = ```:po)
        
            IupSetCallback((ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local h */)).Pointer, "VALUECHANGED_CB", iup_value_cb);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 120, col 5) : ```IupSetCallback($h.Pointer, 'VALUECHANGED_C...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 120 : col 5) | ```IupSetCallba)
        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local h */));

                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 118 : col 18) | (val h = ```:
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 124 : col 1) | (func () (val
                    int ceu_f_iup_dash_timer (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 124 : col 19) | (val h = ```:
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 125 : col 5) | (val h = ```:po
                
                    ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupTimer())} }));
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local h */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 125 : col 5) | (val h = ```:po)
        
            IupSetCallback((ceux_peek(X->S,(X->base + 0 + 0 + 0) /* local h */)).Pointer, "ACTION", (Icallback) iup_list_action_cb);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 126, col 5) : ```IupSetCallback($h.Pointer, 'ACTION', (Ical...)");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 126 : col 5) | ```IupSetCallba)
        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local h */));

                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 124 : col 19) | (val h = ```:
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 133 : col 1) | (func () ```:
                    int ceu_f_iup_dash_hbox (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 133 : col 18) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupHbox(NULL))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 133 : col 18) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 137 : col 1) | (func () ```:
                    int ceu_f_iup_dash_vbox (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 137 : col 18) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupVbox(NULL))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 137 : col 18) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 141 : col 1) | (func (h,child)
                    int ceu_f_iup_dash_append (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 141 : col 28) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupAppend((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,ceux_arg(X, 1) /* arg child */)).Pointer))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 141 : col 28) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 147 : col 1) | (func (h) ```
                    int ceu_f_iup_dash_dialog (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 147 : col 21) | ```:pointer  
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_POINTER, {.Pointer=(  IupDialog((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 147 : col 21) | ```:pointer  
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (@/iup/iup-ceu/iup.ceu : lin 151 : col 1) | (func (h,x,y) 
                    int ceu_f_iup_dash_show_dash_xy (CEUX* X) {
                        
                        do {
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 151 : col 28) | ```:number   
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  IupShowXY((ceux_peek(X->S,ceux_arg(X, 0) /* arg h */)).Pointer, (ceux_peek(X->S,ceux_arg(X, 1) /* arg x */)).Number, (ceux_peek(X->S,ceux_arg(X, 2) /* arg y */)).Number))} }));
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 151 : col 28) | ```:number   
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (01_counter.ceu : lin 27 : col 9) | (task :nested (
                    int ceu_f_17010 (CEUX* X) {
                        
                            X->exe->status = (X->action == CEU_ACTION_ABORT) ? CEU_EXE_STATUS_TERMINATED : CEU_EXE_STATUS_RESUMED;
                            switch (X->exe->pc) {
                                case 0:
                                    if (X->action == CEU_ACTION_ABORT) {
                                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                                        return 1;
                                    }
                        
                        do {
                { // BLOCK | (01_counter.ceu : lin 27 : col 25) | (val button =
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 4 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (01_counter.ceu : lin 4 : col 5) | (val button = i
                
                    
                { // CALL | (01_counter.ceu : lin 4 : col 18) | iup-button(#['C
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 74) /* global iup-button */));

                    
                        
                { // VECTOR | (01_counter.ceu : lin 4 : col 29) | #['C','o','u','
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='C'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='o'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='u'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='n'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='t'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 4, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 4, col 18) : iup-button(#['C','o','u','n','t'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 4 : col 18) | iup-button(#['C
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 0) /* local button */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 4 : col 5) | (val button = i)
        
            
                { // CALL | (01_counter.ceu : lin 5 : col 9) | iup-set(button,
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local button */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_SIZE} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 5 : col 32) | #['6','0']
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='6'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='0'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 5, col 9) : iup-set(button,:SIZE,#['6','0'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 5 : col 9) | iup-set(button,
                
                // DCL | (01_counter.ceu : lin 7 : col 5) | (val text = iup
                
                    
                { // CALL | (01_counter.ceu : lin 7 : col 16) | iup-text()
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 77) /* global iup-text */));

                                        
                    

                    ceux_call(X, 0, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 7, col 16) : iup-text()");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 7 : col 16) | iup-text()
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 1) /* local text */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 7 : col 5) | (val text = iup)
        
            
                { // CALL | (01_counter.ceu : lin 8 : col 9) | iup-set(text,:S
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_SIZE} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 8 : col 34) | #['6','0']
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='6'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='0'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 8, col 9) : iup-set(text,:SIZE,#['6','0'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 8 : col 9) | iup-set(text,:S
                
                { // CALL | (01_counter.ceu : lin 9 : col 9) | iup-set(text,:R
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_READONLY} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 9 : col 34) | #['Y','E','S']
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='Y'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='E'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='S'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 9, col 9) : iup-set(text,:READONLY,#['Y','E','S'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 9 : col 9) | iup-set(text,:R
                
                { // CALL | (01_counter.ceu : lin 10 : col 9) | iup-set(text,:V
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_VALUE} });
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 10, col 9) : iup-set(text,:VALUE,0)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 10 : col 9) | iup-set(text,:V
                
                // DCL | (01_counter.ceu : lin 12 : col 5) | (val hbox = iup
                
                    
                { // CALL | (01_counter.ceu : lin 12 : col 16) | iup-hbox()
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 79) /* global iup-hbox */));

                                        
                    

                    ceux_call(X, 0, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 12, col 16) : iup-hbox()");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 12 : col 16) | iup-hbox()
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 2) /* local hbox */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 12 : col 5) | (val hbox = iup)
        
            
                { // CALL | (01_counter.ceu : lin 13 : col 9) | iup-append(hbox
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 81) /* global iup-append */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local hbox */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                                        
                    

                    ceux_call(X, 2, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 13, col 9) : iup-append(hbox,text)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 13 : col 9) | iup-append(hbox
                
                { // CALL | (01_counter.ceu : lin 14 : col 9) | iup-append(hbox
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 81) /* global iup-append */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local hbox */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local button */));

                                        
                    

                    ceux_call(X, 2, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 14, col 9) : iup-append(hbox,button)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 14 : col 9) | iup-append(hbox
                
                { // CALL | (01_counter.ceu : lin 15 : col 9) | iup-set(hbox,:M
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local hbox */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_MARGIN} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 15 : col 32) | #['1','0','x','
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='1'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='0'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='x'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='1'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='0'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 4, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 15, col 9) : iup-set(hbox,:MARGIN,#['1','0','x','1','0'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 15 : col 9) | iup-set(hbox,:M
                
                { // CALL | (01_counter.ceu : lin 16 : col 9) | iup-set(hbox,:G
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local hbox */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_GAP} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 16 : col 32) | #['1','0']
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='1'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='0'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 16, col 9) : iup-set(hbox,:GAP,#['1','0'])");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 16 : col 9) | iup-set(hbox,:G
                
                // DCL | (01_counter.ceu : lin 18 : col 5) | (val dlg = iup-
                
                    
                { // CALL | (01_counter.ceu : lin 18 : col 15) | iup-dialog(hbox
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 82) /* global iup-dialog */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 2) /* local hbox */));

                                        
                    

                    ceux_call(X, 1, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 18, col 15) : iup-dialog(hbox)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 18 : col 15) | iup-dialog(hbox
                
                    ceux_copy(X->S, (X->base + 0 + 0 + 3) /* local dlg */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 18 : col 5) | (val dlg = iup-)
        
            
                { // CALL | (01_counter.ceu : lin 19 : col 9) | iup-set(dlg,:TI
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 3) /* local dlg */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_TITLE} });
                    
                        
                { // VECTOR | (01_counter.ceu : lin 19 : col 30) | #['C','o','u','
                    ceux_push(X->S, 1, ceu_create_vector());
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='C'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 0, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='o'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 1, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='u'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 2, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='n'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 3, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='t'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 4, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='e'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 5, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_CHAR, {.Char='r'} });
                        ceu_vector_set(&ceux_peek(X->S,XX(-2)).Dyn->Vector, 6, ceux_peek(X->S,XX(-1)));
                        ceux_pop(X->S, 1);
                        
                }
            
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 19, col 9) : iup-set(dlg,:TITLE,#['C','o','u','n','t','e',...)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 19 : col 9) | iup-set(dlg,:TI
                
                { // CALL | (01_counter.ceu : lin 21 : col 5) | iup-show-xy(dlg
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 83) /* global iup-show-xy */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 3) /* local dlg */));

                    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  IUP_CENTER)} }));
                    
                        ceux_push(X->S, 1, ((CEU_Value){ CEU_VALUE_NUMBER, {.Number=(  IUP_CENTER)} }));
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 21, col 5) : iup-show-xy(dlg,```:number   IUP_CENTER```,``...)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 21 : col 5) | iup-show-xy(dlg
                
                // LOOP | (01_counter.ceu : lin 27 : col 21) | loop do (va
                CEU_LOOP_START_17007:
                    
                { // BLOCK | (01_counter.ceu : lin 27 : col 21) | do (var it 
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (01_counter.ceu : lin 28 : col 21) | do (var it :I
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+4, 2 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (01_counter.ceu : lin 30 : col 25) | (var it :Iup.Ac
                
            
                // LOOP | (01_counter.ceu : lin 24 : col 25) | loop (set it 
                CEU_LOOP_START_16961:
                    
                { // BLOCK | (01_counter.ceu : lin 24 : col 25) | (set it = yie
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // SET | (01_counter.ceu : lin 25 : col 29) | (set it = yield
                    
                { // YIELD (01_counter.ceu : lin 28 : col 25) | yield(nil)
                    ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    X->exe->status = CEU_EXE_STATUS_YIELDED;
                    X->exe->pc = 16867;
                    return 1;   // TODO: args MULTI
                case 16867: // YIELD (01_counter.ceu : lin 28 : col 25) | yield(nil)
                    if (X->action == CEU_ACTION_ABORT) {
                        //ceux_push(X->S, 1, (CEU_Value){CEU_VALUE_NIL}); // fake out=1
                        continue;
                    }
                #if CEU >= 4
                    if (X->action == CEU_ACTION_ERROR) {
                        //assert(X->args>1 && CEU_ERROR_IS(X->S) && "TODO: varargs resume");
                        continue;
                    }
                #endif
                    //assert(X->args<=1 && "TODO: varargs resume");
                #if 0
                    // fill missing args with nils
                    {
                        int N = 1 - X->args;
                        assert(N > 0);
                        for (int i=0; i<N; i++) {
                            for (int i=0; i<N; i++) {
                                ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                            }
                        }
                    }
                #endif
                }
              // src is on the stack and should be returned
                    // <<< SRC | DST >>>
                    
                        // ACC - SET | (01_counter.ceu : lin 25 : col 33) | it
                        ceux_repl(X->S, (X->base + 0 + 4 + 0) /* local it */, ceux_peek(X->S,XX(-1)));
                      // dst should not pop src
                }
             // BREAK | (01_counter.ceu : lin 30 : col 33) | (break if do 
                
                { // BLOCK | (01_counter.ceu : lin 28 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+6, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (01_counter.ceu : lin 29 : col 25) | (val ceu_16248 
                
                    
                { // CALL | (01_counter.ceu : lin 27 : col 9) | is'(it,:Iup.Act
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iup_Action} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 27, col 9) : is'(it,:Iup.Action)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 27 : col 9) | is'(it,:Iup.Act
                
                    ceux_copy(X->S, (X->base + 0 + 6 + 0) /* local ceu_16248 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 29 : col 25) | (val ceu_16248 )
        
            
                { // IF | (01_counter.ceu : lin 28 : col 25) | if ceu_16248 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 6 + 0) /* local ceu_16248 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (01_counter.ceu : lin 28 : col 38) | do (val ceu
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // BLOCK | (01_counter.ceu : lin 24 : col 21) | do (val ceu_1
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 1 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                // DCL | (01_counter.ceu : lin 25 : col 25) | (val ceu_16062 
                
                    
                { // CALL | (01_counter.ceu : lin 23 : col 39) | is'(it,:Iup.Act
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_Iup_Action} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 23, col 39) : is'(it,:Iup.Action)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 23 : col 39) | is'(it,:Iup.Act
                
                    ceux_copy(X->S, (X->base + 0 + 7 + 0) /* local ceu_16062 */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 25 : col 25) | (val ceu_16062 )
        
            
                { // IF | (01_counter.ceu : lin 24 : col 25) | if ceu_16062 
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 7 + 0) /* local ceu_16062 */));

                    {
                        int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        ceux_pop(X->S, 1);
                        if (v) {
                            
                { // BLOCK | (01_counter.ceu : lin 24 : col 38) | ==(it[:h]
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+8, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        
                { // CALL | (01_counter.ceu : lin 23 : col 29) | ==(it[:h],b
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 14) /* global {{==}} */));

                    
                        
                { // INDEX | (01_counter.ceu : lin 23 : col 25) | it[:h]
                    // IDX
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=0} });
                        
                    
                    // COL
                    ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 0) /* local it */));

                    CEU_ERROR_CHK_VAL (
                        continue,
                        ceu_col_check(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))),
                        "01_counter.ceu : (lin 23, col 25) : it[:h]"
                    );
                
                        {
                            CEU_Value v = CEU_ERROR_CHK_VAL(continue, ceu_col_get(ceux_peek(X->S,XX(-1)),ceux_peek(X->S,XX(-2))), "01_counter.ceu : (lin 23, col 25) : it[:h]");
                            ceu_gc_inc_val(v);
                            ceux_drop(X->S, 2);
                            ceux_push(X->S, 1, v);
                            ceu_gc_dec_val(v);
                        }
                    
                }
                
                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 0) /* local button */));

                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 23, col 29) : {{==}}(it[:h],button)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 23 : col 29) | ==(it[:h],b
                
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 24 : col 38) | ==(it[:h]
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+8, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        } else {
                            
                { // BLOCK | (01_counter.ceu : lin 24 : col 32) | ceu_16062
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+8, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 7 + 0) /* local ceu_16062 */));

                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 24 : col 32) | ceu_16062
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+8, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 24 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 28 : col 38) | do (val ceu
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        } else {
                            
                { // BLOCK | (01_counter.ceu : lin 28 : col 32) | ceu_16248
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+7, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 6 + 0) /* local ceu_16248 */));

                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 28 : col 32) | ceu_16248
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+7, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        }
                    }
                }
                
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 28 : col 21) | do (val ceu_1
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+6, 1, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                {
                    int v = ceu_as_bool(ceux_peek(X->S, XX(-1)));
                        // pop condition:
                        //  1. when false, clear for next iteration
                        //  2. when true,  but return e is given
                        //  3. when true,  but ret=0
                    if (!v) {
                        ceux_pop(X->S, 1);            // (1)
                    } else {
                        
                            
                        
                        CEU_BREAK = 1;
                        goto CEU_LOOP_STOP_16961;
                    }
                }
            
                        
                            CEU_LOOP_STOP_16961:
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 24 : col 25) | (set it = yie
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  1;
                        ceux_block_leave(X->S, X->base+6, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_16961;
                    }
            // DELAY | (01_counter.ceu : lin 34 : col 25) | delay
X->exe_task->time = CEU_TIME;
                // DCL | (01_counter.ceu : lin 24 : col 9) | (val v = iup-ge
                
                    
                { // CALL | (01_counter.ceu : lin 24 : col 17) | iup-get-number(
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 72) /* global iup-get-number */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_VALUE} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 24, col 17) : iup-get-number(text,:VALUE)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 24 : col 17) | iup-get-number(
                
                    ceux_copy(X->S, (X->base + 0 + 4 + 1) /* local v */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((01_counter.ceu : lin 24 : col 9) | (val v = iup-ge)
        
            
                { // CALL | (01_counter.ceu : lin 25 : col 9) | iup-set(text,:V
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 0 + 1) /* local text */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_TAG, {.Tag=CEU_TAG_VALUE} });
                    
                        
                { // CALL | (01_counter.ceu : lin 25 : col 32) | +(v,1)
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */));

                    
                        ceux_push(X->S, 1, ceux_peek(X->S, (X->base + 0 + 4 + 1) /* local v */));

                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NUMBER, {.Number=1} });
                                        
                    

                    ceux_call(X, 2, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 25, col 32) : {{+}}(v,1)");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 25 : col 32) | +(v,1)
                
                                        
                    

                    ceux_call(X, 3, 0);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 25, col 9) : iup-set(text,:VALUE,{{+}}(v,1))");
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                } // CALL | (01_counter.ceu : lin 25 : col 9) | iup-set(text,:V
                
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 28 : col 21) | do (var it :I
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+4, 2, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                        
                            CEU_LOOP_STOP_17007:
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 27 : col 21) | do (var it 
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : ) !CEU_BREAK ? 0 :  1;
                        ceux_block_leave(X->S, X->base+4, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                
                    if (CEU_BREAK) {
                        CEU_BREAK = 0;
                    } else {
                        //ceux_pop(X->S, 1);
                        goto CEU_LOOP_START_17007;
                    }
            
                        
                    } while (0);

                    // BLOCK (escape) | (01_counter.ceu : lin 27 : col 25) | (val button =
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 4, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
                if (X->exe->status == CEU_EXE_STATUS_TERMINATED) {
                    continue;
                }
            
        
                }
                } while (0);
                        
                                {
                                    int top = ceux_n_get(X->S);
                                
                                    // task return value in pub(t)
                                    if (top > 0) {
                                        ceu_gc_dec_val(X->exe_task->pub);
                                        X->exe_task->pub = ceux_peek(X->S, XX(-1));
                                        ceu_gc_inc_val(X->exe_task->pub);
                                    }
                                
                                    int ret = ceu_exe_term(X);
                                    if (!CEU_ERROR_IS(X->S) && ret!=0) {
                                        // nrm->err: remove pending return in the stack
                                        ceux_rem(X->S, top-1);
                                    }
                                }
                            } // close switch
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
                    // PROTO | (anon : lin 0 : col 0) | (func () (val
                    int ceu_f_17089 (CEUX* X) {
                        
                        do {
                { // BLOCK | (anon : lin 0 : col 0) | (val dump)(v
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+0, 85 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    
                    {
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_dump_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 0, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_error_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 1, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_next_dash_dict_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 2, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_print_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 3, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_println_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 4, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_sup_question__f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 5, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_tags_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 6, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_to_dash_string_dash_number_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 7, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_to_dash_string_dash_pointer_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 8, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_to_dash_string_dash_tag_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 9, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_to_dash_tag_dash_string_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 10, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_tuple_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 11, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_type_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 12, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_hash_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 13, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_equals_equals_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 14, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_slash_equals_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 15, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_coroutine_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 16, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_status_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 17, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_broadcast_plic__f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 18, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_next_dash_tasks_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 19, clo);
                        }
                        
                        {
                            CEU_Value clo = ceu_create_clo(CEU_VALUE_CLO_FUNC, ceu_tasks_f, 0, 0, 0);
                            ceux_repl(X->S, X->base + 20, clo);
                        }
                        
                    }
                    

                    // defers init
                    
                    ceux_repl(X->S, (1 + 0 + 84) /* global defer */, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=0} });
                        // false: not reached, dont finalize
                
                    
                    do {    
                        
                // DCL | (anon : lin 0 : col 0) | (val dump)
                
            
                // DCL | (anon : lin 0 : col 0) | (val error)
                
            
                // DCL | (anon : lin 0 : col 0) | (val next-dict)
                
            
                // DCL | (anon : lin 0 : col 0) | (val print)
                
            
                // DCL | (anon : lin 0 : col 0) | (val println)
                
            
                // DCL | (anon : lin 0 : col 0) | (val sup?)
                
            
                // DCL | (anon : lin 0 : col 0) | (val tags)
                
            
                // DCL | (anon : lin 0 : col 0) | (val to-string-
                
            
                // DCL | (anon : lin 0 : col 0) | (val to-string-
                
            
                // DCL | (anon : lin 0 : col 0) | (val to-string-
                
            
                // DCL | (anon : lin 0 : col 0) | (val to-tag-str
                
            
                // DCL | (anon : lin 0 : col 0) | (val tuple)
                
            
                // DCL | (anon : lin 0 : col 0) | (val type)
                
            
                // DCL | (anon : lin 0 : col 0) | (val #)
                
            
                // DCL | (anon : lin 0 : col 0) | (val ==)
                
            
                // DCL | (anon : lin 0 : col 0) | (val /=)
                
            
                // DCL | (anon : lin 0 : col 0) | (val coroutine)
                
            
                // DCL | (anon : lin 0 : col 0) | (val status)
                
            
                // DCL | (anon : lin 0 : col 0) | (val broadcast'
                
            
                // DCL | (anon : lin 0 : col 0) | (val next-tasks
                
            
                // DCL | (anon : lin 0 : col 0) | (val tasks)
                
            // DATA | (prelude.ceu : lin 1 : col 6) | (data :Clock = 

                // DCL | (prelude.ceu : lin 3 : col 1) | (val debug = (f
                
                     // CREATE | (prelude.ceu : lin 3 : col 1) | (func (v) pri
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_debug,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 21) /* global debug */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 3 : col 1) | (val debug = (f)
        
            
                // DCL | (prelude.ceu : lin 12 : col 5) | (val + = (f
                
                     // CREATE | (prelude.ceu : lin 12 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_plus,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 22) /* global {{+}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 12 : col 5) | (val + = (f)
        
            
                // DCL | (prelude.ceu : lin 19 : col 5) | (val - = (f
                
                     // CREATE | (prelude.ceu : lin 19 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_minus,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 23) /* global {{-}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 19 : col 5) | (val - = (f)
        
            
                // DCL | (prelude.ceu : lin 27 : col 5) | (val * = (f
                
                     // CREATE | (prelude.ceu : lin 27 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_asterisk,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 24) /* global {{*}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 27 : col 5) | (val * = (f)
        
            
                // DCL | (prelude.ceu : lin 34 : col 5) | (val ** = (
                
                     // CREATE | (prelude.ceu : lin 34 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_asterisk_asterisk,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 25) /* global {{**}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 34 : col 5) | (val ** = ()
        
            
                // DCL | (prelude.ceu : lin 41 : col 5) | (val / = (f
                
                     // CREATE | (prelude.ceu : lin 41 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_slash,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 26) /* global {{/}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 41 : col 5) | (val / = (f)
        
            
                // DCL | (prelude.ceu : lin 48 : col 5) | (val // = (
                
                     // CREATE | (prelude.ceu : lin 48 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_slash_slash,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 27) /* global {{//}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 48 : col 5) | (val // = ()
        
            
                // DCL | (prelude.ceu : lin 55 : col 5) | (val % = (f
                
                     // CREATE | (prelude.ceu : lin 55 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_null,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 28) /* global {{%}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 55 : col 5) | (val % = (f)
        
            
                // DCL | (prelude.ceu : lin 66 : col 5) | (val > = (f
                
                     // CREATE | (prelude.ceu : lin 66 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_greater,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 29) /* global {{>}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 66 : col 5) | (val > = (f)
        
            
                // DCL | (prelude.ceu : lin 74 : col 5) | (val < = (f
                
                     // CREATE | (prelude.ceu : lin 74 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_less,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 30) /* global {{<}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 74 : col 5) | (val < = (f)
        
            
                // DCL | (prelude.ceu : lin 78 : col 5) | (val >= = (
                
                     // CREATE | (prelude.ceu : lin 78 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_greater_equals,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 31) /* global {{>=}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 78 : col 5) | (val >= = ()
        
            
                // DCL | (prelude.ceu : lin 82 : col 5) | (val <= = (
                
                     // CREATE | (prelude.ceu : lin 82 : col 5) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_less_equals,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 32) /* global {{<=}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 82 : col 5) | (val <= = ()
        
            
                // DCL | (prelude.ceu : lin 89 : col 1) | (val assert = (
                
                     // CREATE | (prelude.ceu : lin 89 : col 1) | (func (v,msg) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_assert,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 33) /* global assert */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 89 : col 1) | (val assert = ()
        
            
                // DCL | (prelude.ceu : lin 102 : col 1) | (val tag-or = (
                
                     // CREATE | (prelude.ceu : lin 102 : col 1) | (func (t1,t2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_tag_dash_or,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 34) /* global tag-or */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 102 : col 1) | (val tag-or = ()
        
            
                // DCL | (prelude.ceu : lin 112 : col 1) | (val min = (fun
                
                     // CREATE | (prelude.ceu : lin 112 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_min,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 35) /* global min */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 112 : col 1) | (val min = (fun)
        
            
                // DCL | (prelude.ceu : lin 116 : col 1) | (val max = (fun
                
                     // CREATE | (prelude.ceu : lin 116 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_max,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 36) /* global max */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 116 : col 1) | (val max = (fun)
        
            
                // DCL | (prelude.ceu : lin 120 : col 1) | (val between = 
                
                     // CREATE | (prelude.ceu : lin 120 : col 1) | (func (min_max,
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_between,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 37) /* global between */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 120 : col 1) | (val between = )
        
            // DATA | (prelude.ceu : lin 128 : col 6) | (data :Iterator

                // DCL | (prelude.ceu : lin 131 : col 5) | (val iter-tuple
                
                     // CREATE | (prelude.ceu : lin 131 : col 5) | (func (itr :Ite
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iter_dash_tuple,
                        1,  // TODO: remove assert
                        6,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 38) /* global iter-tuple */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 131 : col 5) | (val iter-tuple)
        
            
                // DCL | (prelude.ceu : lin 147 : col 5) | (val iter-dict 
                
                     // CREATE | (prelude.ceu : lin 147 : col 5) | (func (itr :Ite
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iter_dash_dict,
                        1,  // TODO: remove assert
                        6,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 39) /* global iter-dict */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 147 : col 5) | (val iter-dict )
        
            
                // DCL | (prelude.ceu : lin 163 : col 5) | (val iter-xcoro
                
                     // CREATE | (prelude.ceu : lin 163 : col 5) | (func (itr :Ite
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iter_dash_xcoro,
                        1,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 40) /* global iter-xcoro */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 163 : col 5) | (val iter-xcoro)
        
            
                // DCL | (prelude.ceu : lin 172 : col 5) | (val iter-coro 
                
                     // CREATE | (prelude.ceu : lin 172 : col 5) | (func (itr :Ite
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iter_dash_coro,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 41) /* global iter-coro */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 172 : col 5) | (val iter-coro )
        
            
                // DCL | (prelude.ceu : lin 178 : col 5) | (val iter-tasks
                
                     // CREATE | (prelude.ceu : lin 178 : col 5) | (func (itr :Ite
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iter_dash_tasks,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 42) /* global iter-tasks */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 178 : col 5) | (val iter-tasks)
        
            
                // DCL | (prelude.ceu : lin 184 : col 5) | (val to-iter = 
                
                     // CREATE | (prelude.ceu : lin 184 : col 5) | (func (v,tp) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_iter,
                        2,  // TODO: remove assert
                        9,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 43) /* global to-iter */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 184 : col 5) | (val to-iter = )
        
            
                // DCL | (prelude.ceu : lin 213 : col 1) | (val === = 
                
                     // CREATE | (prelude.ceu : lin 213 : col 1) | (func :rec (v1,
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_equals_equals_equals,
                        2,  // TODO: remove assert
                        7,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 44) /* global {{===}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 213 : col 1) | (val === = )
        
            
                // DCL | (prelude.ceu : lin 240 : col 1) | (val =/= = 
                
                     // CREATE | (prelude.ceu : lin 240 : col 1) | (func :rec (v1,
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_equals_slash_equals,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 45) /* global {{=/=}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 240 : col 1) | (val =/= = )
        
            
                // DCL | (prelude.ceu : lin 244 : col 1) | (val is' = (fun
                
                     // CREATE | (prelude.ceu : lin 244 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_is_plic_,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 46) /* global is' */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 244 : col 1) | (val is' = (fun)
        
            
                // DCL | (prelude.ceu : lin 254 : col 1) | (val is-not' = 
                
                     // CREATE | (prelude.ceu : lin 254 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_is_dash_not_plic_,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 47) /* global is-not' */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 254 : col 1) | (val is-not' = )
        
            
                // DCL | (prelude.ceu : lin 258 : col 1) | (val in' = (fun
                
                     // CREATE | (prelude.ceu : lin 258 : col 1) | (func (v,xs) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_in_plic_,
                        2,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 48) /* global in' */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 258 : col 1) | (val in' = (fun)
        
            
                // DCL | (prelude.ceu : lin 264 : col 1) | (val in-not' = 
                
                     // CREATE | (prelude.ceu : lin 264 : col 1) | (func (v,xs) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_in_dash_not_plic_,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 49) /* global in-not' */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 264 : col 1) | (val in-not' = )
        
            
                // DCL | (prelude.ceu : lin 272 : col 1) | (val string? = 
                
                     // CREATE | (prelude.ceu : lin 272 : col 1) | (func (v) do 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_string_question_,
                        1,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 50) /* global string? */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 272 : col 1) | (val string? = )
        
            
                // DCL | (prelude.ceu : lin 276 : col 1) | (val to-string 
                
                     // CREATE | (prelude.ceu : lin 276 : col 1) | (func :rec (v) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_string,
                        1,  // TODO: remove assert
                        7,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 51) /* global to-string */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 276 : col 1) | (val to-string )
        
            
                // DCL | (prelude.ceu : lin 288 : col 1) | (val to-bool = 
                
                     // CREATE | (prelude.ceu : lin 288 : col 1) | (func (v) if 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_bool,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 52) /* global to-bool */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 288 : col 1) | (val to-bool = )
        
            
                // DCL | (prelude.ceu : lin 292 : col 1) | (val to-number 
                
                     // CREATE | (prelude.ceu : lin 292 : col 1) | (func (v) do 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_number,
                        1,  // TODO: remove assert
                        4,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 53) /* global to-number */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 292 : col 1) | (val to-number )
        
            
                // DCL | (prelude.ceu : lin 301 : col 1) | (val to-tag = (
                
                     // CREATE | (prelude.ceu : lin 301 : col 1) | (func (v) do 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_tag,
                        1,  // TODO: remove assert
                        3,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 54) /* global to-tag */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 301 : col 1) | (val to-tag = ()
        
            
                // DCL | (prelude.ceu : lin 309 : col 1) | (val to-pointer
                
                     // CREATE | (prelude.ceu : lin 309 : col 1) | (func (v) do 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_pointer,
                        1,  // TODO: remove assert
                        6,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 55) /* global to-pointer */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 309 : col 1) | (val to-pointer)
        
            
                // DCL | (prelude.ceu : lin 324 : col 1) | (val type-stati
                
                     // CREATE | (prelude.ceu : lin 324 : col 1) | (func (v) <
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_type_dash_static_question_,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 56) /* global type-static? */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 324 : col 1) | (val type-stati)
        
            
                // DCL | (prelude.ceu : lin 327 : col 1) | (val type-dynam
                
                     // CREATE | (prelude.ceu : lin 327 : col 1) | (func (v) >
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_type_dash_dynamic_question_,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 57) /* global type-dynamic? */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 327 : col 1) | (val type-dynam)
        
            
                // DCL | (prelude.ceu : lin 333 : col 1) | (val copy = (fu
                
                     // CREATE | (prelude.ceu : lin 333 : col 1) | (func :rec (v) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_copy,
                        1,  // TODO: remove assert
                        7,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 58) /* global copy */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 333 : col 1) | (val copy = (fu)
        
            
                // DCL | (prelude.ceu : lin 351 : col 1) | (val to-vector 
                
                     // CREATE | (prelude.ceu : lin 351 : col 1) | (func (col,tp) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_vector,
                        2,  // TODO: remove assert
                        4,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 59) /* global to-vector */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 351 : col 1) | (val to-vector )
        
            
                // DCL | (prelude.ceu : lin 379 : col 1) | (val to-set = (
                
                     // CREATE | (prelude.ceu : lin 379 : col 1) | (func (col) (
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_to_dash_set,
                        1,  // TODO: remove assert
                        3,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 60) /* global to-set */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 379 : col 1) | (val to-set = ()
        
            
                // DCL | (prelude.ceu : lin 391 : col 1) | (val <++ = 
                
                     // CREATE | (prelude.ceu : lin 391 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_less_plus_plus,
                        2,  // TODO: remove assert
                        4,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 61) /* global {{<++}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 391 : col 1) | (val <++ = )
        
            
                // DCL | (prelude.ceu : lin 402 : col 1) | (val ++ = (
                
                     // CREATE | (prelude.ceu : lin 402 : col 1) | (func (v1,v2) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_plus_plus,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 62) /* global {{++}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 402 : col 1) | (val ++ = ()
        
            
                // DCL | (prelude.ceu : lin 408 : col 1) | (val <|< = 
                
                     // CREATE | (prelude.ceu : lin 408 : col 1) | (func (f,g) (
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_less_bar_less,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 63) /* global {{<|<}} */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 408 : col 1) | (val <|< = )
        
            
                // DCL | (prelude.ceu : lin 418 : col 1) | (val random-see
                
                     // CREATE | (prelude.ceu : lin 418 : col 1) | (func (n) if 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_random_dash_seed,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 64) /* global random-seed */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 418 : col 1) | (val random-see)
        
            
                // DCL | (prelude.ceu : lin 426 : col 1) | (val random-nex
                
                     // CREATE | (prelude.ceu : lin 426 : col 1) | (func () ```:
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_random_dash_next,
                        0,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 65) /* global random-next */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 426 : col 1) | (val random-nex)
        
            
                // DCL | (prelude.ceu : lin 430 : col 1) | (val math-sin =
                
                     // CREATE | (prelude.ceu : lin 430 : col 1) | (func (v) ```
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_math_dash_sin,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 66) /* global math-sin */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 430 : col 1) | (val math-sin =)
        
            
                // DCL | (prelude.ceu : lin 434 : col 1) | (val math-cos =
                
                     // CREATE | (prelude.ceu : lin 434 : col 1) | (func (v) ```
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_math_dash_cos,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 67) /* global math-cos */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((prelude.ceu : lin 434 : col 1) | (val math-cos =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 438 : col 1) | (val math-floor
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 438 : col 1) | (func (v) ```
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_math_dash_floor,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 68) /* global math-floor */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 438 : col 1) | (val math-floor)
        
            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 1 : col 1) | do (data :Iup
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+85, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        // DATA | (@/iup/iup-ceu/iup.ceu : lin 1 : col 6) | (data :Iup = [h
// DATA | (@/iup/iup-ceu/iup.ceu : lin 2 : col 5) | (data :Iup.Acti
// DATA | (@/iup/iup-ceu/iup.ceu : lin 3 : col 5) | (data :Iup.Valu
// DATA | (@/iup/iup-ceu/iup.ceu : lin 4 : col 5) | (data :Iup.List
// DATA | (@/iup/iup-ceu/iup.ceu : lin 5 : col 9) | (data :Iup.List

                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 1 : col 1) | do (data :Iup
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  0;
                        ceux_block_leave(X->S, X->base+85, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 9 : col 1) | ```:pre #inclu)
        
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 61 : col 1) | (val iup-loop =
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 61 : col 1) | (func () ```I
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_loop,
                        0,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 69) /* global iup-loop */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 61 : col 1) | (val iup-loop =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 67 : col 1) | (val iup-get-po
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 67 : col 1) | (func (h,name) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_get_dash_pointer,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 70) /* global iup-get-pointer */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 67 : col 1) | (val iup-get-po)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 71 : col 1) | (val iup-get-bo
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 71 : col 1) | (func (h,name) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_get_dash_bool,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 71) /* global iup-get-bool */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 71 : col 1) | (val iup-get-bo)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 75 : col 1) | (val iup-get-nu
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 75 : col 1) | (func (h,name) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_get_dash_number,
                        2,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 72) /* global iup-get-number */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 75 : col 1) | (val iup-get-nu)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 80 : col 1) | (val iup-set = 
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 80 : col 1) | (func (h,name,v
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_set,
                        3,  // TODO: remove assert
                        6,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 73) /* global iup-set */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 80 : col 1) | (val iup-set = )
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 100 : col 1) | (val iup-button
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 100 : col 1) | (func (label) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_button,
                        1,  // TODO: remove assert
                        2,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 74) /* global iup-button */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 100 : col 1) | (val iup-button)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 107 : col 1) | (val iup-label 
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 107 : col 1) | (func (label) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_label,
                        1,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 75) /* global iup-label */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 107 : col 1) | (val iup-label )
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 112 : col 1) | (val iup-list =
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 112 : col 1) | (func () (val
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_list,
                        0,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 76) /* global iup-list */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 112 : col 1) | (val iup-list =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 118 : col 1) | (val iup-text =
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 118 : col 1) | (func () (val
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_text,
                        0,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 77) /* global iup-text */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 118 : col 1) | (val iup-text =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 124 : col 1) | (val iup-timer 
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 124 : col 1) | (func () (val
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_timer,
                        0,  // TODO: remove assert
                        1,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 78) /* global iup-timer */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 124 : col 1) | (val iup-timer )
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 133 : col 1) | (val iup-hbox =
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 133 : col 1) | (func () ```:
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_hbox,
                        0,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 79) /* global iup-hbox */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 133 : col 1) | (val iup-hbox =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 137 : col 1) | (val iup-vbox =
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 137 : col 1) | (func () ```:
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_vbox,
                        0,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 80) /* global iup-vbox */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 137 : col 1) | (val iup-vbox =)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 141 : col 1) | (val iup-append
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 141 : col 1) | (func (h,child)
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_append,
                        2,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 81) /* global iup-append */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 141 : col 1) | (val iup-append)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 147 : col 1) | (val iup-dialog
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 147 : col 1) | (func (h) ```
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_dialog,
                        1,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 82) /* global iup-dialog */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 147 : col 1) | (val iup-dialog)
        
            
                // DCL | (@/iup/iup-ceu/iup.ceu : lin 151 : col 1) | (val iup-show-x
                
                     // CREATE | (@/iup/iup-ceu/iup.ceu : lin 151 : col 1) | (func (h,x,y) 
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_iup_dash_show_dash_xy,
                        3,  // TODO: remove assert
                        0,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                    ceux_copy(CEU_GLOBAL_X->S, (1 + 0 + 83) /* global iup-show-xy */, XX(-1));
                    
                    // recursive func requires its self ref upv to be reset to itself
                    
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 151 : col 1) | (val iup-show-x)
        
            IupOpen(NULL,NULL);

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 157, col 1) : ```IupOpen(NULL,NULL);```");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
            ceux_pop(X->S,1); // PI0 ((@/iup/iup-ceu/iup.ceu : lin 157 : col 1) | ```IupOpen(NULL)
        
                ceux_repl(X->S, (1 + 0 + 84) /* global defer */, (CEU_Value) { CEU_VALUE_BOOL, {.Bool=1} });
                        // true: reached, finalize
                ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                
                { // SPAWN | (01_counter.ceu : lin 27 : col 2) | (spawn (task :n
                    
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                     // CREATE | (01_counter.ceu : lin 27 : col 9) | (task :nested (
                {
                    
                    CEU_Value clo = ceu_create_clo_task (
                        
                        ceu_f_17010,
                        0,  // TODO: remove assert
                        8,
                        0
                        , NULL
                    );
                    ceux_push(X->S, 1, clo);
                    
                        // TODO: use args+locs+upvs+tmps?
                        //clo.Dyn->Clo_Exe.mem_n = sizeof(CEU_Clo_Mem_17010);                    
                    
                    
                    // UPVALS = 0
                    
                }
                
                    
                    {
                        ceux_spawn(X, 0, X->now);
                        
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 27, col 2) : (spawn (task :nested () { (val button = iup-b...)");
            
        
                        ceux_pop(X->S, 1);
                    }
                } // SPAWN | (01_counter.ceu : lin 27 : col 2) | (spawn (task :n
            
                { // CALL | (01_counter.ceu : lin 29 : col 1) | iup-loop()
                    ceux_push(X->S, 1, ceux_peek(CEU_GLOBAL_X->S, (1 + 0 + 69) /* global iup-loop */));

                                        
                    

                    ceux_call(X, 0, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "01_counter.ceu : (lin 29, col 1) : iup-loop()");
            
        
                } // CALL | (01_counter.ceu : lin 29 : col 1) | iup-loop()
                
                        
                    } while (0);

                    // BLOCK (escape) | (anon : lin 0 : col 0) | (val dump)(v
                    // defers execute
                    
                    if (ceux_peek(X->S,(1 + 0 + 84) /* global defer */).Bool) {     // if true: reached, finalize
                        do {
                            
                { // BLOCK | (@/iup/iup-ceu/iup.ceu : lin 158 : col 7) | ```IupClose()
                    // do not clear upvs
                    ceux_block_enter(X->S, X->base+85, 0 CEU4(COMMA X->exe));
                    
                    // GLOBALS (must be after ceux_block_enter)
                    

                    // defers init
                    
                    
                    do {    
                        IupClose();

                        
            CEU_ERROR_CHK_STK(continue, "@/iup/iup-ceu/iup.ceu : (lin 159, col 5) : ```IupClose();```");
            
        
                        ceux_push(X->S, 1, (CEU_Value) { CEU_VALUE_NIL });
                    
                        
                    } while (0);

                    // BLOCK (escape) | (@/iup/iup-ceu/iup.ceu : lin 158 : col 7) | ```IupClose()
                    // defers execute
                    
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+85, 0, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                
                        } while (0);    // catch throw
                        assert(!CEU_ERROR_IS(X->S) && "TODO: error in defer");
                        ceux_pop(X->S, 1);
                    }
                
                    
                    // out=0 when loop iterates (!CEU_BREAK)
                    {
                        int out = CEU3(X->action==CEU_ACTION_ABORT ? 0 : )  1;
                        ceux_block_leave(X->S, X->base+0, 85, out);
                    }
                    
                    
            CEU_ERROR_CHK_STK(continue, NULL);
            
        
                }
                } while (0);
                        
                        return CEU3(X->action==CEU_ACTION_ABORT ? 0 :) 1;
                    }
                
    
    int main (int ceu_argc, char** ceu_argv) {
        assert(CEU_TAG_nil == CEU_VALUE_NIL);
        
    #if 0
        // ... args ...
        {
            CEU_Value xxx = ceu_create_tuple(ceu_argc);
            for (int i=0; i<ceu_argc; i++) {
                CEU_Value vec = ceu_vector_from_c_string(ceu_argv[i]);
                ceu_tuple_set(&xxx.Dyn->Tuple, i, vec);
            }
            ceux_push(X->S, 1, xxx);
        }
    #endif
    
        CEU_Stack S = { 0, {} };
        CEUX _X = { &S, -1, -1 CEU3(COMMA CEU_ACTION_INVALID COMMA {.exe=NULL}) CEU4(COMMA CEU_TIME COMMA NULL) };
        CEUX* X = &_X;
        CEU_GLOBAL_X = X;
        
        do {
                { // CALL | (anon : lin 0 : col 0) | (func () (val
                     // CREATE | (anon : lin 0 : col 0) | (func () (val
                {
                    
                    CEU_Value clo = ceu_create_clo (
                        CEU_VALUE_CLO_FUNC,
                        ceu_f_17089,
                        0,  // TODO: remove assert
                        94,
                        0
                        
                    );
                    ceux_push(X->S, 1, clo);
                    
                    
                    // UPVALS = 0
                    
                }
                
                                        
                    

                    ceux_call(X, 0, 1);
                    
                    
            CEU_ERROR_CHK_STK(continue, "anon : (lin 0, col 0) : (func () { (val dump) (val error) (val next-d...)");
            
        
                } // CALL | (anon : lin 0 : col 0) | (func () (val
                } while (0);

        // uncaught throw
    #if CEU >= 2
        if (CEU_ERROR_IS(X->S)) {
            // [...,n,pay,err]
            CEU_Value n = ceux_peek(X->S, XX(-3));
            assert(n.type == CEU_VALUE_NUMBER);
            // ignore i=0 (main call)
            for (int i=1; i<n.Number; i++) {
                printf(" |  ");
                CEU_Value pre = ceux_peek(X->S, XX(-4-i));
                assert(pre.type==CEU_VALUE_POINTER && pre.Pointer!=NULL);
                printf("%s\n", (char*) pre.Pointer);
            }
            CEU_Value pay = ceux_peek(X->S, XX(-2));
            if (pay.type == CEU_VALUE_POINTER) {
                assert(pay.Pointer != NULL);
                printf(" v  %s\n", (char*) pay.Pointer);     // payload is primitive error
            } else {
                printf(" v  error : ");
                ceu_print1(ceux_peek(X->S, XX(-2)));
                puts("");
            }
        }
    #endif

        ceux_n_set(X->S, 0);
        return 0;
    }
    