; ModuleID = '../architecture/scheduler.cpp'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.6.0"

%struct.thread_time_constraint_policy = type { i32, i32, i32, i32 }
%struct.mach_timebase_info = type { i32, i32 }
%struct._opaque_pthread_t = type { i64, %struct.__darwin_pthread_handler_rec*, [1168 x i8] }
%struct.__darwin_pthread_handler_rec = type { void (i8*)*, i8*, %struct.__darwin_pthread_handler_rec* }
%class.DSPThreadPool = type { %class.DSPThread**, i32, i32 }
%class.DSPThread = type { i32 (...)**, %struct._opaque_pthread_t*, %class.DSPThreadPool*, %class.Semaphore, i8, i32, i8* }
%class.Semaphore = type { i32 }
%struct.thread_affinity_policy = type { i32 }
%struct._opaque_pthread_attr_t = type { i64, [56 x i8] }
%struct.sched_param = type { i32, [4 x i8] }
%class.WorkStealingScheduler = type { %class.DSPThreadPool*, %class.TaskQueue*, %class.TaskGraph*, %class.DynThreadAdapter, i32, i32, i32*, i32, i32 }
%class.TaskQueue = type { i32*, i32, %struct.AtomicCounter, i64, i64 }
%struct.AtomicCounter = type { %union.anon }
%union.anon = type { i32 }
%class.TaskGraph = type { i32*, i32 }
%class.DynThreadAdapter = type { [50 x i64], i64, i64, i32, float, i32, i8 }

@_ZL10gTimeRatio = internal unnamed_addr global double 0.000000e+00, align 8
@_ZL7gPeriod = internal global i64 0, align 8
@_ZL12gComputation = internal global i64 0, align 8
@_ZL11gConstraint = internal global i64 0, align 8
@.str = private unnamed_addr constant [15 x i8] c"hw.physicalcpu\00", align 1
@.str1 = private unnamed_addr constant [14 x i8] c"hw.logicalcpu\00", align 1
@.str2 = private unnamed_addr constant [16 x i8] c"OMP_NUM_THREADS\00", align 1
@.str3 = private unnamed_addr constant [13 x i8] c"CLOCKSPERSEC\00", align 1
@.str4 = private unnamed_addr constant [17 x i8] c"OMP_STEALING_DUR\00", align 1
@.str5 = private unnamed_addr constant [15 x i8] c"OMP_DYN_THREAD\00", align 1
@.str6 = private unnamed_addr constant [13 x i8] c"OMP_REALTIME\00", align 1
@.str7 = private unnamed_addr constant [80 x i8] c"Cannot request joinable thread creation for real-time thread res = %d err = %s\0A\00", align 1
@.str8 = private unnamed_addr constant [68 x i8] c"Cannot set scheduling scope for real-time thread res = %d err = %s\0A\00", align 1
@.str9 = private unnamed_addr constant [68 x i8] c"Cannot request explicit scheduling for RT thread res = %d err = %s\0A\00", align 1
@.str10 = private unnamed_addr constant [64 x i8] c"Cannot set RR scheduling class for RT thread res = %d err = %s\0A\00", align 1
@.str11 = private unnamed_addr constant [64 x i8] c"Cannot set scheduling priority for RT thread res = %d err = %s\0A\00", align 1
@.str12 = private unnamed_addr constant [48 x i8] c"Cannot set thread stack size res = %d err = %s\0A\00", align 1
@.str13 = private unnamed_addr constant [40 x i8] c"Cannot create thread res = %d err = %s\0A\00", align 1
@_ZTV9DSPThread = linkonce_odr unnamed_addr constant [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI9DSPThread to i8*), i8* bitcast (void (%class.DSPThread*)* @_ZN9DSPThreadD1Ev to i8*), i8* bitcast (void (%class.DSPThread*)* @_ZN9DSPThreadD0Ev to i8*)]
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS9DSPThread = linkonce_odr constant [11 x i8] c"9DSPThread\00"
@_ZTI9DSPThread = linkonce_odr unnamed_addr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([11 x i8]* @_ZTS9DSPThread, i32 0, i32 0) }
@mach_task_self_ = external global i32
@_ZTIi = external constant i8*

define i64 @_Z15GetMicroSecondsv() uwtable ssp {
entry:
  %call = tail call i64 @mach_absolute_time()
  %conv = uitofp i64 %call to double
  %0 = load double* @_ZL10gTimeRatio, align 8, !tbaa !0
  %mul = fmul double %conv, %0
  %conv1 = fptoui double %mul to i64
  ret i64 %conv1
}

declare i64 @mach_absolute_time()

define void @_Z11GetRealTimev() uwtable ssp {
entry:
  %theTCPolicy.i = alloca %struct.thread_time_constraint_policy, align 4
  %count.i = alloca i32, align 4
  %get_default.i = alloca i32, align 4
  %info.i = alloca %struct.mach_timebase_info, align 4
  %0 = load i64* @_ZL7gPeriod, align 8, !tbaa !3
  %cmp = icmp eq i64 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = bitcast %struct.mach_timebase_info* %info.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %1)
  %call.i = call i32 @mach_timebase_info(%struct.mach_timebase_info* %info.i)
  %numer.i = getelementptr inbounds %struct.mach_timebase_info* %info.i, i64 0, i32 0
  %2 = load i32* %numer.i, align 4, !tbaa !4
  %conv.i = uitofp i32 %2 to double
  %denom.i = getelementptr inbounds %struct.mach_timebase_info* %info.i, i64 0, i32 1
  %3 = load i32* %denom.i, align 4, !tbaa !4
  %conv1.i = uitofp i32 %3 to double
  %div.i = fdiv double %conv.i, %conv1.i
  %div2.i = fdiv double %div.i, 1.000000e+03
  store double %div2.i, double* @_ZL10gTimeRatio, align 8, !tbaa !0
  call void @llvm.lifetime.end(i64 -1, i8* %1)
  %call = call %struct._opaque_pthread_t* @pthread_self()
  %4 = bitcast %struct.thread_time_constraint_policy* %theTCPolicy.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %4)
  %5 = bitcast i32* %count.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %5)
  %6 = bitcast i32* %get_default.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %6)
  store i32 4, i32* %count.i, align 4, !tbaa !4
  store i32 0, i32* %get_default.i, align 4, !tbaa !4
  %call.i2 = call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %call)
  %7 = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i, i64 0, i32 0
  %call1.i = call i32 @thread_policy_get(i32 %call.i2, i32 2, i32* %7, i32* %count.i, i32* %get_default.i)
  %cmp.i = icmp eq i32 %call1.i, 0
  br i1 %cmp.i, label %if.then.i, label %if.end

if.then.i:                                        ; preds = %if.then
  %8 = load i32* %7, align 4, !tbaa !4
  %conv.i3 = zext i32 %8 to i64
  store i64 %conv.i3, i64* @_ZL7gPeriod, align 8, !tbaa !3
  %computation3.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i, i64 0, i32 1
  %9 = load i32* %computation3.i, align 4, !tbaa !4
  %conv4.i = zext i32 %9 to i64
  store i64 %conv4.i, i64* @_ZL12gComputation, align 8, !tbaa !3
  %constraint5.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i, i64 0, i32 2
  %10 = load i32* %constraint5.i, align 4, !tbaa !4
  %conv6.i = zext i32 %10 to i64
  store i64 %conv6.i, i64* @_ZL11gConstraint, align 8, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.then.i, %if.then, %entry
  ret void
}

declare %struct._opaque_pthread_t* @pthread_self()

define i32 @_Z11get_max_cpuv() uwtable ssp {
entry:
  %physical_count = alloca i32, align 4
  %size = alloca i64, align 8
  %logical_count = alloca i32, align 4
  store i32 0, i32* %physical_count, align 4, !tbaa !4
  store i64 4, i64* %size, align 8, !tbaa !5
  %0 = bitcast i32* %physical_count to i8*
  %call = call i32 @sysctlbyname(i8* getelementptr inbounds ([15 x i8]* @.str, i64 0, i64 0), i8* %0, i64* %size, i8* null, i64 0)
  store i32 0, i32* %logical_count, align 4, !tbaa !4
  %1 = bitcast i32* %logical_count to i8*
  %call1 = call i32 @sysctlbyname(i8* getelementptr inbounds ([14 x i8]* @.str1, i64 0, i64 0), i8* %1, i64* %size, i8* null, i64 0)
  %2 = load i32* %physical_count, align 4, !tbaa !4
  ret i32 %2
}

declare i32 @sysctlbyname(i8*, i8*, i64*, i8*, i64)

define void @_ZN13DSPThreadPoolC1Ei(%class.DSPThreadPool* nocapture %this, i32 %thread_pool_size) unnamed_addr uwtable ssp align 2 {
entry:
  %0 = sext i32 %thread_pool_size to i64
  %1 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %0, i64 8)
  %2 = extractvalue { i64, i1 } %1, 1
  %3 = extractvalue { i64, i1 } %1, 0
  %4 = select i1 %2, i64 -1, i64 %3
  %call.i = tail call noalias i8* @_Znam(i64 %4)
  %5 = bitcast i8* %call.i to %class.DSPThread**
  %fThreadPool.i = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  store %class.DSPThread** %5, %class.DSPThread*** %fThreadPool.i, align 8, !tbaa !6
  %cmp3.i = icmp sgt i32 %thread_pool_size, 0
  br i1 %cmp3.i, label %for.body.i.preheader, label %_ZN13DSPThreadPoolC2Ei.exit

for.body.i.preheader:                             ; preds = %entry
  store %class.DSPThread* null, %class.DSPThread** %5, align 8, !tbaa !6
  %exitcond.i2 = icmp eq i32 %thread_pool_size, 1
  br i1 %exitcond.i2, label %_ZN13DSPThreadPoolC2Ei.exit, label %for.body.for.body_crit_edge.i

for.body.for.body_crit_edge.i:                    ; preds = %for.body.i.preheader, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge
  %.pre.i = phi %class.DSPThread** [ %.pre.i.pre, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge ], [ %5, %for.body.i.preheader ]
  %indvars.iv.next.i3 = phi i64 [ %indvars.iv.next.i, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge ], [ 1, %for.body.i.preheader ]
  %arrayidx.i = getelementptr inbounds %class.DSPThread** %.pre.i, i64 %indvars.iv.next.i3
  store %class.DSPThread* null, %class.DSPThread** %arrayidx.i, align 8, !tbaa !6
  %indvars.iv.next.i = add i64 %indvars.iv.next.i3, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %thread_pool_size
  br i1 %exitcond, label %_ZN13DSPThreadPoolC2Ei.exit, label %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge

for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge: ; preds = %for.body.for.body_crit_edge.i
  %.pre.i.pre = load %class.DSPThread*** %fThreadPool.i, align 8, !tbaa !6
  br label %for.body.for.body_crit_edge.i

_ZN13DSPThreadPoolC2Ei.exit:                      ; preds = %for.body.i.preheader, %for.body.for.body_crit_edge.i, %entry
  %fThreadCount.i = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 1
  store i32 0, i32* %fThreadCount.i, align 4, !tbaa !4
  %fCurThreadCount.i = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 2
  store volatile i32 0, i32* %fCurThreadCount.i, align 4, !tbaa !4
  ret void
}

define void @_ZN13DSPThreadPoolC2Ei(%class.DSPThreadPool* nocapture %this, i32 %thread_pool_size) unnamed_addr uwtable ssp align 2 {
entry:
  %0 = sext i32 %thread_pool_size to i64
  %1 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %0, i64 8)
  %2 = extractvalue { i64, i1 } %1, 1
  %3 = extractvalue { i64, i1 } %1, 0
  %4 = select i1 %2, i64 -1, i64 %3
  %call = tail call noalias i8* @_Znam(i64 %4)
  %5 = bitcast i8* %call to %class.DSPThread**
  %fThreadPool = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  store %class.DSPThread** %5, %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %cmp3 = icmp sgt i32 %thread_pool_size, 0
  br i1 %cmp3, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body.for.body_crit_edge
  %6 = phi %class.DSPThread** [ %.pre, %for.body.for.body_crit_edge ], [ %5, %entry ]
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.for.body_crit_edge ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds %class.DSPThread** %6, i64 %indvars.iv
  store %class.DSPThread* null, %class.DSPThread** %arrayidx, align 8, !tbaa !6
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %thread_pool_size
  br i1 %exitcond, label %for.end, label %for.body.for.body_crit_edge

for.body.for.body_crit_edge:                      ; preds = %for.body
  %.pre = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  br label %for.body

for.end:                                          ; preds = %for.body, %entry
  %fThreadCount = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 1
  store i32 0, i32* %fThreadCount, align 4, !tbaa !4
  %fCurThreadCount = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 2
  store volatile i32 0, i32* %fCurThreadCount, align 4, !tbaa !4
  ret void
}

declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) nounwind readnone

declare noalias i8* @_Znam(i64)

define void @_ZN13DSPThreadPoolD1Ev(%class.DSPThreadPool* nocapture %this) unnamed_addr uwtable ssp align 2 {
entry:
  tail call void @_ZN13DSPThreadPoolD2Ev(%class.DSPThreadPool* %this)
  ret void
}

define void @_ZN13DSPThreadPoolD2Ev(%class.DSPThreadPool* nocapture %this) unnamed_addr uwtable ssp align 2 {
entry:
  %fThreadCount.i = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 1
  %0 = load i32* %fThreadCount.i, align 4, !tbaa !4
  %cmp2.i = icmp sgt i32 %0, 0
  br i1 %cmp2.i, label %for.body.lr.ph.i, label %for.cond.preheader.for.end_crit_edge

for.body.lr.ph.i:                                 ; preds = %entry
  %fThreadPool.i = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ 0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %for.body.i ]
  %1 = load %class.DSPThread*** %fThreadPool.i, align 8, !tbaa !6
  %arrayidx.i = getelementptr inbounds %class.DSPThread** %1, i64 %indvars.iv.i
  %2 = load %class.DSPThread** %arrayidx.i, align 8, !tbaa !6
  %fThread.i.i = getelementptr inbounds %class.DSPThread* %2, i64 0, i32 1
  %3 = load %struct._opaque_pthread_t** %fThread.i.i, align 8, !tbaa !6
  %call.i.i.i = tail call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %3)
  %call1.i.i.i = tail call i32 @thread_terminate(i32 %call.i.i.i)
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %4 = load i32* %fThreadCount.i, align 4, !tbaa !4
  %5 = trunc i64 %indvars.iv.next.i to i32
  %cmp.i = icmp slt i32 %5, %4
  br i1 %cmp.i, label %for.body.i, label %for.cond.preheader

for.cond.preheader:                               ; preds = %for.body.i
  %cmp10 = icmp sgt i32 %4, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.cond.preheader.for.end_crit_edge

for.cond.preheader.for.end_crit_edge:             ; preds = %entry, %for.cond.preheader
  %fThreadPool6.pre = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.end

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %fThreadPool = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %delete.end
  %6 = phi i32 [ %4, %for.body.lr.ph ], [ %11, %delete.end ]
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %delete.end ]
  %7 = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %arrayidx = getelementptr inbounds %class.DSPThread** %7, i64 %indvars.iv
  %8 = load %class.DSPThread** %arrayidx, align 8, !tbaa !6
  %isnull = icmp eq %class.DSPThread* %8, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %for.body
  %9 = bitcast %class.DSPThread* %8 to void (%class.DSPThread*)***
  %vtable = load void (%class.DSPThread*)*** %9, align 8, !tbaa !7
  %vfn = getelementptr inbounds void (%class.DSPThread*)** %vtable, i64 1
  %10 = load void (%class.DSPThread*)** %vfn, align 8
  tail call void %10(%class.DSPThread* %8)
  %.pre = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %.pre12 = load i32* %fThreadCount.i, align 4, !tbaa !4
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %for.body
  %11 = phi i32 [ %.pre12, %delete.notnull ], [ %6, %for.body ]
  %12 = phi %class.DSPThread** [ %.pre, %delete.notnull ], [ %7, %for.body ]
  %arrayidx4 = getelementptr inbounds %class.DSPThread** %12, i64 %indvars.iv
  store %class.DSPThread* null, %class.DSPThread** %arrayidx4, align 8, !tbaa !6
  %indvars.iv.next = add i64 %indvars.iv, 1
  %13 = trunc i64 %indvars.iv.next to i32
  %cmp = icmp slt i32 %13, %11
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %delete.end, %for.cond.preheader.for.end_crit_edge
  %fThreadPool6.pre-phi = phi %class.DSPThread*** [ %fThreadPool6.pre, %for.cond.preheader.for.end_crit_edge ], [ %fThreadPool, %delete.end ]
  store i32 0, i32* %fThreadCount.i, align 4, !tbaa !4
  %14 = load %class.DSPThread*** %fThreadPool6.pre-phi, align 8, !tbaa !6
  %isnull7 = icmp eq %class.DSPThread** %14, null
  br i1 %isnull7, label %delete.end9, label %delete.notnull8

delete.notnull8:                                  ; preds = %for.end
  %15 = bitcast %class.DSPThread** %14 to i8*
  tail call void @_ZdaPv(i8* %15) nounwind
  br label %delete.end9

delete.end9:                                      ; preds = %delete.notnull8, %for.end
  ret void
}

define void @_ZN13DSPThreadPool7StopAllEv(%class.DSPThreadPool* nocapture %this) uwtable ssp align 2 {
entry:
  %fThreadCount = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 1
  %0 = load i32* %fThreadCount, align 4, !tbaa !4
  %cmp2 = icmp sgt i32 %0, 0
  br i1 %cmp2, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %fThreadPool = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %1 = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %arrayidx = getelementptr inbounds %class.DSPThread** %1, i64 %indvars.iv
  %2 = load %class.DSPThread** %arrayidx, align 8, !tbaa !6
  %fThread.i = getelementptr inbounds %class.DSPThread* %2, i64 0, i32 1
  %3 = load %struct._opaque_pthread_t** %fThread.i, align 8, !tbaa !6
  %call.i.i = tail call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %3)
  %call1.i.i = tail call i32 @thread_terminate(i32 %call.i.i)
  %indvars.iv.next = add i64 %indvars.iv, 1
  %4 = load i32* %fThreadCount, align 4, !tbaa !4
  %5 = trunc i64 %indvars.iv.next to i32
  %cmp = icmp slt i32 %5, %4
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare void @_ZdaPv(i8*) nounwind

define void @_ZN13DSPThreadPool8StartAllEibPv(%class.DSPThreadPool* %this, i32 %num_thread, i1 zeroext %realtime, i8* %dsp) uwtable ssp align 2 {
entry:
  %fThreadCount = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 1
  %0 = load i32* %fThreadCount, align 4, !tbaa !4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %for.cond.preheader, label %if.end

for.cond.preheader:                               ; preds = %entry
  %fThreadPool = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.cond

for.cond:                                         ; preds = %for.cond.preheader, %invoke.cont
  %indvars.iv = phi i64 [ 0, %for.cond.preheader ], [ %indvars.iv.next, %invoke.cont ]
  %1 = trunc i64 %indvars.iv to i32
  %cmp2 = icmp slt i32 %1, %num_thread
  br i1 %cmp2, label %for.body, label %if.end

for.body:                                         ; preds = %for.cond
  %call = tail call noalias i8* @_Znwm(i64 48)
  %2 = bitcast i8* %call to %class.DSPThread*
  %3 = bitcast i8* %call to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ([4 x i8*]* @_ZTV9DSPThread, i64 0, i64 2) to i32 (...)**), i32 (...)*** %3, align 8, !tbaa !7
  %fThreadPool.i.i = getelementptr inbounds i8* %call, i64 16
  %4 = bitcast i8* %fThreadPool.i.i to %class.DSPThreadPool**
  store %class.DSPThreadPool* %this, %class.DSPThreadPool** %4, align 8, !tbaa !6
  %5 = load i32* @mach_task_self_, align 4, !tbaa !4
  %_sem.i.i.i.i = getelementptr inbounds i8* %call, i64 24
  %6 = bitcast i8* %_sem.i.i.i.i to i32*
  %call.i.i.i.i10 = invoke i32 @semaphore_create(i32 %5, i32* %6, i32 0, i32 0)
          to label %call.i.i.i.i.noexc unwind label %lpad.loopexit

call.i.i.i.i.noexc:                               ; preds = %for.body
  %tobool.i.i.i.i = icmp eq i32 %call.i.i.i.i10, 0
  br i1 %tobool.i.i.i.i, label %invoke.cont, label %if.then.i.i.i.i

if.then.i.i.i.i:                                  ; preds = %call.i.i.i.i.noexc
  %exception.i.i.i.i = tail call i8* @__cxa_allocate_exception(i64 4) nounwind
  %7 = bitcast i8* %exception.i.i.i.i to i32*
  store i32 -1, i32* %7, align 4, !tbaa !4
  invoke void @__cxa_throw(i8* %exception.i.i.i.i, i8* bitcast (i8** @_ZTIi to i8*), i8* null) noreturn
          to label %.noexc unwind label %lpad.nonloopexit

.noexc:                                           ; preds = %if.then.i.i.i.i
  unreachable

invoke.cont:                                      ; preds = %call.i.i.i.i.noexc
  %8 = getelementptr inbounds i8* %call, i64 28
  store i8 0, i8* %8, align 1, !tbaa !8
  %fNumThread.i.i = getelementptr inbounds i8* %call, i64 32
  %9 = bitcast i8* %fNumThread.i.i to i32*
  store i32 %1, i32* %9, align 4, !tbaa !4
  %fDSP.i.i = getelementptr inbounds i8* %call, i64 40
  %10 = bitcast i8* %fDSP.i.i to i8**
  store i8* %dsp, i8** %10, align 8, !tbaa !6
  %11 = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %arrayidx = getelementptr inbounds %class.DSPThread** %11, i64 %indvars.iv
  store %class.DSPThread* %2, %class.DSPThread** %arrayidx, align 8, !tbaa !6
  %12 = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %arrayidx5 = getelementptr inbounds %class.DSPThread** %12, i64 %indvars.iv
  %13 = load %class.DSPThread** %arrayidx5, align 8, !tbaa !6
  %call6 = tail call i32 @_ZN9DSPThread5StartEb(%class.DSPThread* %13, i1 zeroext %realtime)
  %14 = load i32* %fThreadCount, align 4, !tbaa !4
  %inc = add nsw i32 %14, 1
  store i32 %inc, i32* %fThreadCount, align 4, !tbaa !4
  %indvars.iv.next = add i64 %indvars.iv, 1
  br label %for.cond

lpad.loopexit:                                    ; preds = %for.body
  %lpad.loopexit11 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  br label %lpad

lpad.nonloopexit:                                 ; preds = %if.then.i.i.i.i
  %lpad.nonloopexit12 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  br label %lpad

lpad:                                             ; preds = %lpad.nonloopexit, %lpad.loopexit
  %lpad.phi = phi { i8*, i32 } [ %lpad.loopexit11, %lpad.loopexit ], [ %lpad.nonloopexit12, %lpad.nonloopexit ]
  tail call void @_ZdlPv(i8* %call) nounwind
  resume { i8*, i32 } %lpad.phi

if.end:                                           ; preds = %for.cond, %entry
  ret void
}

declare noalias i8* @_Znwm(i64)

declare i32 @__gxx_personality_v0(...)

declare void @_ZdlPv(i8*) nounwind

define linkonce_odr i32 @_ZN9DSPThread5StartEb(%class.DSPThread* %this, i1 zeroext %realtime) uwtable ssp align 2 {
entry:
  %theTCPolicy.i = alloca %struct.thread_affinity_policy, align 4
  %attributes = alloca %struct._opaque_pthread_attr_t, align 8
  %rt_param = alloca i64, align 8
  %tmpcast = bitcast i64* %rt_param to %struct.sched_param*
  %call = call i32 @pthread_attr_init(%struct._opaque_pthread_attr_t* %attributes)
  br i1 %realtime, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %fRealTime = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 4
  store i8 1, i8* %fRealTime, align 1, !tbaa !8
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = call i8* @getenv(i8* getelementptr inbounds ([13 x i8]* @.str6, i64 0, i64 0))
  %tobool3 = icmp eq i8* %call2, null
  br i1 %tobool3, label %cond.end, label %cond.true

cond.true:                                        ; preds = %if.else
  %call5 = call i64 @strtol(i8* nocapture %call2, i8** null, i32 10)
  %phitmp = icmp ne i64 %call5, 0
  br label %cond.end

cond.end:                                         ; preds = %if.else, %cond.true
  %cond = phi i1 [ %phitmp, %cond.true ], [ true, %if.else ]
  %fRealTime7 = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 4
  %frombool8 = zext i1 %cond to i8
  store i8 %frombool8, i8* %fRealTime7, align 1, !tbaa !8
  br label %if.end

if.end:                                           ; preds = %cond.end, %if.then
  %call9 = call i32 @pthread_attr_setdetachstate(%struct._opaque_pthread_attr_t* %attributes, i32 1)
  %tobool10 = icmp eq i32 %call9, 0
  br i1 %tobool10, label %if.end15, label %if.then11

if.then11:                                        ; preds = %if.end
  %call12 = call i32* @__error()
  %0 = load i32* %call12, align 4, !tbaa !4
  %call13 = call i8* @"\01_strerror"(i32 %0)
  %call14 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([80 x i8]* @.str7, i64 0, i64 0), i32 %call9, i8* %call13)
  br label %return

if.end15:                                         ; preds = %if.end
  %call16 = call i32 @pthread_attr_setscope(%struct._opaque_pthread_attr_t* %attributes, i32 1)
  %tobool17 = icmp eq i32 %call16, 0
  br i1 %tobool17, label %if.end22, label %if.then18

if.then18:                                        ; preds = %if.end15
  %call19 = call i32* @__error()
  %1 = load i32* %call19, align 4, !tbaa !4
  %call20 = call i8* @"\01_strerror"(i32 %1)
  %call21 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([68 x i8]* @.str8, i64 0, i64 0), i32 %call16, i8* %call20)
  br label %return

if.end22:                                         ; preds = %if.end15
  br i1 %realtime, label %if.then24, label %if.else46

if.then24:                                        ; preds = %if.end22
  %call25 = call i32 @pthread_attr_setinheritsched(%struct._opaque_pthread_attr_t* %attributes, i32 2)
  %tobool26 = icmp eq i32 %call25, 0
  br i1 %tobool26, label %if.end31, label %if.then27

if.then27:                                        ; preds = %if.then24
  %call28 = call i32* @__error()
  %2 = load i32* %call28, align 4, !tbaa !4
  %call29 = call i8* @"\01_strerror"(i32 %2)
  %call30 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([68 x i8]* @.str9, i64 0, i64 0), i32 %call25, i8* %call29)
  br label %return

if.end31:                                         ; preds = %if.then24
  %call32 = call i32 @pthread_attr_setschedpolicy(%struct._opaque_pthread_attr_t* %attributes, i32 4)
  %tobool33 = icmp eq i32 %call32, 0
  br i1 %tobool33, label %if.end38, label %if.then34

if.then34:                                        ; preds = %if.end31
  %call35 = call i32* @__error()
  %3 = load i32* %call35, align 4, !tbaa !4
  %call36 = call i8* @"\01_strerror"(i32 %3)
  %call37 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([64 x i8]* @.str10, i64 0, i64 0), i32 %call32, i8* %call36)
  br label %return

if.end38:                                         ; preds = %if.end31
  store i64 0, i64* %rt_param, align 8
  %sched_priority = bitcast i64* %rt_param to i32*
  store i32 60, i32* %sched_priority, align 8, !tbaa !4
  %call39 = call i32 @pthread_attr_setschedparam(%struct._opaque_pthread_attr_t* %attributes, %struct.sched_param* %tmpcast)
  %tobool40 = icmp eq i32 %call39, 0
  br i1 %tobool40, label %if.end54, label %if.then41

if.then41:                                        ; preds = %if.end38
  %call42 = call i32* @__error()
  %4 = load i32* %call42, align 4, !tbaa !4
  %call43 = call i8* @"\01_strerror"(i32 %4)
  %call44 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([64 x i8]* @.str11, i64 0, i64 0), i32 %call39, i8* %call43)
  br label %return

if.else46:                                        ; preds = %if.end22
  %call47 = call i32 @pthread_attr_setinheritsched(%struct._opaque_pthread_attr_t* %attributes, i32 1)
  %tobool48 = icmp eq i32 %call47, 0
  br i1 %tobool48, label %if.end54, label %if.then49

if.then49:                                        ; preds = %if.else46
  %call50 = call i32* @__error()
  %5 = load i32* %call50, align 4, !tbaa !4
  %call51 = call i8* @"\01_strerror"(i32 %5)
  %call52 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([68 x i8]* @.str9, i64 0, i64 0), i32 %call47, i8* %call51)
  br label %return

if.end54:                                         ; preds = %if.end38, %if.else46
  %call55 = call i32 @pthread_attr_setstacksize(%struct._opaque_pthread_attr_t* %attributes, i64 524288)
  %tobool56 = icmp eq i32 %call55, 0
  br i1 %tobool56, label %if.end61, label %if.then57

if.then57:                                        ; preds = %if.end54
  %call58 = call i32* @__error()
  %6 = load i32* %call58, align 4, !tbaa !4
  %call59 = call i8* @"\01_strerror"(i32 %6)
  %call60 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([48 x i8]* @.str12, i64 0, i64 0), i32 %call55, i8* %call59)
  br label %return

if.end61:                                         ; preds = %if.end54
  %fThread = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 1
  %7 = bitcast %class.DSPThread* %this to i8*
  %call62 = call i32 @pthread_create(%struct._opaque_pthread_t** %fThread, %struct._opaque_pthread_attr_t* %attributes, i8* (i8*)* @_ZN9DSPThread13ThreadHandlerEPv, i8* %7)
  %tobool63 = icmp eq i32 %call62, 0
  br i1 %tobool63, label %if.end68, label %if.then64

if.then64:                                        ; preds = %if.end61
  %call65 = call i32* @__error()
  %8 = load i32* %call65, align 4, !tbaa !4
  %call66 = call i8* @"\01_strerror"(i32 %8)
  %call67 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([40 x i8]* @.str13, i64 0, i64 0), i32 %call62, i8* %call66)
  br label %return

if.end68:                                         ; preds = %if.end61
  %9 = load %struct._opaque_pthread_t** %fThread, align 8, !tbaa !6
  %fNumThread = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 5
  %10 = load i32* %fNumThread, align 4, !tbaa !4
  %add = add nsw i32 %10, 1
  %11 = bitcast %struct.thread_affinity_policy* %theTCPolicy.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %11)
  %affinity_tag.i = getelementptr inbounds %struct.thread_affinity_policy* %theTCPolicy.i, i64 0, i32 0
  store i32 %add, i32* %affinity_tag.i, align 4, !tbaa !4
  %call.i = call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %9)
  %call1.i = call i32 @thread_policy_set(i32 %call.i, i32 4, i32* %affinity_tag.i, i32 1)
  call void @llvm.lifetime.end(i64 -1, i8* %11)
  %call70 = call i32 @pthread_attr_destroy(%struct._opaque_pthread_attr_t* %attributes)
  br label %return

return:                                           ; preds = %if.end68, %if.then64, %if.then57, %if.then49, %if.then41, %if.then34, %if.then27, %if.then18, %if.then11
  %retval.0 = phi i32 [ -1, %if.then11 ], [ -1, %if.then18 ], [ -1, %if.then27 ], [ -1, %if.then34 ], [ -1, %if.then41 ], [ -1, %if.then57 ], [ -1, %if.then64 ], [ 0, %if.end68 ], [ -1, %if.then49 ]
  ret i32 %retval.0
}

define void @_ZN13DSPThreadPool9SignalAllEi(%class.DSPThreadPool* nocapture %this, i32 %num_thread) uwtable ssp align 2 {
entry:
  %fCurThreadCount = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 2
  store volatile i32 %num_thread, i32* %fCurThreadCount, align 4, !tbaa !4
  %cmp2 = icmp sgt i32 %num_thread, 0
  br i1 %cmp2, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %fThreadPool = getelementptr inbounds %class.DSPThreadPool* %this, i64 0, i32 0
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %0 = load %class.DSPThread*** %fThreadPool, align 8, !tbaa !6
  %arrayidx = getelementptr inbounds %class.DSPThread** %0, i64 %indvars.iv
  %1 = load %class.DSPThread** %arrayidx, align 8, !tbaa !6
  %_sem.i.i = getelementptr inbounds %class.DSPThread* %1, i64 0, i32 3, i32 0
  %2 = load i32* %_sem.i.i, align 4, !tbaa !4
  %call.i.i = tail call i32 @semaphore_signal(i32 %2)
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %num_thread
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

define i8* @createScheduler(i32 %task_queue_size, i32 %init_task_list_size) uwtable ssp {
entry:
  %call = tail call noalias i8* @_Znwm(i64 480)
  %0 = bitcast i8* %call to %class.WorkStealingScheduler*
  invoke void @_ZN21WorkStealingSchedulerC2Eii(%class.WorkStealingScheduler* %0, i32 %task_queue_size, i32 %init_task_list_size)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  ret i8* %call

lpad:                                             ; preds = %entry
  %1 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  tail call void @_ZdlPv(i8* %call) nounwind
  resume { i8*, i32 } %1
}

define void @deleteScheduler(i8* %scheduler) uwtable ssp {
entry:
  %isnull = icmp eq i8* %scheduler, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  %0 = bitcast i8* %scheduler to %class.WorkStealingScheduler*
  invoke void @_ZN21WorkStealingSchedulerD2Ev(%class.WorkStealingScheduler* %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %delete.notnull
  tail call void @_ZdlPv(i8* %scheduler) nounwind
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont, %entry
  ret void

lpad:                                             ; preds = %delete.notnull
  %1 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  tail call void @_ZdlPv(i8* %scheduler) nounwind
  resume { i8*, i32 } %1
}

define void @startAll(i8* nocapture %scheduler, i8* %dsp) uwtable ssp {
entry:
  %fThreadPool.i = bitcast i8* %scheduler to %class.DSPThreadPool**
  %0 = load %class.DSPThreadPool** %fThreadPool.i, align 8, !tbaa !6
  %fStaticNumThreads.i = getelementptr inbounds i8* %scheduler, i64 460
  %1 = bitcast i8* %fStaticNumThreads.i to i32*
  %2 = load i32* %1, align 4, !tbaa !4
  %sub.i = add nsw i32 %2, -1
  tail call void @_ZN13DSPThreadPool8StartAllEibPv(%class.DSPThreadPool* %0, i32 %sub.i, i1 zeroext true, i8* %dsp)
  ret void
}

define void @stopAll(i8* nocapture %scheduler) uwtable ssp {
entry:
  %fThreadPool.i = bitcast i8* %scheduler to %class.DSPThreadPool**
  %0 = load %class.DSPThreadPool** %fThreadPool.i, align 8, !tbaa !6
  %fThreadCount.i.i = getelementptr inbounds %class.DSPThreadPool* %0, i64 0, i32 1
  %1 = load i32* %fThreadCount.i.i, align 4, !tbaa !4
  %cmp2.i.i = icmp sgt i32 %1, 0
  br i1 %cmp2.i.i, label %for.body.lr.ph.i.i, label %_ZN21WorkStealingScheduler7StopAllEv.exit

for.body.lr.ph.i.i:                               ; preds = %entry
  %fThreadPool.i.i = getelementptr inbounds %class.DSPThreadPool* %0, i64 0, i32 0
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i
  %indvars.iv.i.i = phi i64 [ 0, %for.body.lr.ph.i.i ], [ %indvars.iv.next.i.i, %for.body.i.i ]
  %2 = load %class.DSPThread*** %fThreadPool.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds %class.DSPThread** %2, i64 %indvars.iv.i.i
  %3 = load %class.DSPThread** %arrayidx.i.i, align 8, !tbaa !6
  %fThread.i.i.i = getelementptr inbounds %class.DSPThread* %3, i64 0, i32 1
  %4 = load %struct._opaque_pthread_t** %fThread.i.i.i, align 8, !tbaa !6
  %call.i.i.i.i = tail call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %4)
  %call1.i.i.i.i = tail call i32 @thread_terminate(i32 %call.i.i.i.i)
  %indvars.iv.next.i.i = add i64 %indvars.iv.i.i, 1
  %5 = load i32* %fThreadCount.i.i, align 4, !tbaa !4
  %6 = trunc i64 %indvars.iv.next.i.i to i32
  %cmp.i.i = icmp slt i32 %6, %5
  br i1 %cmp.i.i, label %for.body.i.i, label %_ZN21WorkStealingScheduler7StopAllEv.exit

_ZN21WorkStealingScheduler7StopAllEv.exit:        ; preds = %for.body.i.i, %entry
  ret void
}

define void @signalAll(i8* nocapture %scheduler) uwtable ssp {
entry:
  tail call void @_Z11GetRealTimev()
  %0 = getelementptr inbounds i8* %scheduler, i64 452
  %1 = load i8* %0, align 1, !tbaa !8, !range !9
  %tobool.i.i = icmp eq i8 %1, 0
  br i1 %tobool.i.i, label %_ZN16DynThreadAdapter12StartMeasureEv.exit.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %entry
  %call.i.i.i = tail call i64 @mach_absolute_time()
  %conv.i.i.i = uitofp i64 %call.i.i.i to double
  %2 = load double* @_ZL10gTimeRatio, align 8, !tbaa !0
  %mul.i.i.i = fmul double %conv.i.i.i, %2
  %conv1.i.i.i = fptoui double %mul.i.i.i to i64
  %fStart.i.i = getelementptr inbounds i8* %scheduler, i64 424
  %3 = bitcast i8* %fStart.i.i to i64*
  store i64 %conv1.i.i.i, i64* %3, align 8, !tbaa !3
  br label %_ZN16DynThreadAdapter12StartMeasureEv.exit.i

_ZN16DynThreadAdapter12StartMeasureEv.exit.i:     ; preds = %if.then.i.i, %entry
  %fThreadPool.i = bitcast i8* %scheduler to %class.DSPThreadPool**
  %4 = load %class.DSPThreadPool** %fThreadPool.i, align 8, !tbaa !6
  %fDynamicNumThreads.i = getelementptr inbounds i8* %scheduler, i64 456
  %5 = bitcast i8* %fDynamicNumThreads.i to i32*
  %6 = load i32* %5, align 4, !tbaa !4
  %sub.i = add nsw i32 %6, -1
  %fCurThreadCount.i.i = getelementptr inbounds %class.DSPThreadPool* %4, i64 0, i32 2
  store volatile i32 %sub.i, i32* %fCurThreadCount.i.i, align 4, !tbaa !4
  %cmp2.i.i = icmp sgt i32 %sub.i, 0
  br i1 %cmp2.i.i, label %for.body.lr.ph.i.i, label %_ZN21WorkStealingScheduler9SignalAllEv.exit

for.body.lr.ph.i.i:                               ; preds = %_ZN16DynThreadAdapter12StartMeasureEv.exit.i
  %fThreadPool.i.i = getelementptr inbounds %class.DSPThreadPool* %4, i64 0, i32 0
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i
  %indvars.iv.i.i = phi i64 [ 0, %for.body.lr.ph.i.i ], [ %indvars.iv.next.i.i, %for.body.i.i ]
  %7 = load %class.DSPThread*** %fThreadPool.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds %class.DSPThread** %7, i64 %indvars.iv.i.i
  %8 = load %class.DSPThread** %arrayidx.i.i, align 8, !tbaa !6
  %_sem.i.i.i.i = getelementptr inbounds %class.DSPThread* %8, i64 0, i32 3, i32 0
  %9 = load i32* %_sem.i.i.i.i, align 4, !tbaa !4
  %call.i.i.i.i = tail call i32 @semaphore_signal(i32 %9)
  %indvars.iv.next.i.i = add i64 %indvars.iv.i.i, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next.i.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %sub.i
  br i1 %exitcond, label %_ZN21WorkStealingScheduler9SignalAllEv.exit, label %for.body.i.i

_ZN21WorkStealingScheduler9SignalAllEv.exit:      ; preds = %for.body.i.i, %_ZN16DynThreadAdapter12StartMeasureEv.exit.i
  ret void
}

define void @syncAll(i8* nocapture %scheduler) uwtable ssp {
entry:
  %fDynThreadAdapter.i = getelementptr inbounds i8* %scheduler, i64 24
  %0 = bitcast i8* %fDynThreadAdapter.i to %class.DynThreadAdapter*
  %fStaticNumThreads.i = getelementptr inbounds i8* %scheduler, i64 460
  %1 = bitcast i8* %fStaticNumThreads.i to i32*
  %2 = load i32* %1, align 4, !tbaa !4
  %fDynamicNumThreads.i = getelementptr inbounds i8* %scheduler, i64 456
  %3 = bitcast i8* %fDynamicNumThreads.i to i32*
  tail call void @_ZN16DynThreadAdapter11StopMeasureEiRi(%class.DynThreadAdapter* %0, i32 %2, i32* %3)
  ret void
}

define void @pushHead(i8* nocapture %scheduler, i32 %cur_thread, i32 %task_num) nounwind uwtable ssp {
entry:
  %idxprom.i = sext i32 %cur_thread to i64
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %0 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %1 = load %class.TaskQueue** %0, align 8, !tbaa !6
  %info.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %idxprom.i, i32 2, i32 0
  %fHead.i.i = bitcast %union.anon* %info.i.i to i16*
  %2 = load volatile i16* %fHead.i.i, align 2, !tbaa !10
  %idxprom.i.i = sext i16 %2 to i64
  %fTaskList.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %idxprom.i, i32 0
  %3 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds i32* %3, i64 %idxprom.i.i
  store i32 %task_num, i32* %arrayidx.i.i, align 4, !tbaa !4
  %4 = load volatile i16* %fHead.i.i, align 2, !tbaa !10
  %inc.i.i = add i16 %4, 1
  store volatile i16 %inc.i.i, i16* %fHead.i.i, align 2, !tbaa !10
  ret void
}

define i32 @getNextTask(i8* nocapture %scheduler, i32 %cur_thread) nounwind uwtable ssp {
entry:
  %value.addr.i.i.i.i = alloca i32, align 4
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %0 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %1 = load %class.TaskQueue** %0, align 8, !tbaa !6
  %fDynamicNumThreads.i = getelementptr inbounds i8* %scheduler, i64 456
  %2 = bitcast i8* %fDynamicNumThreads.i to i32*
  %3 = load i32* %2, align 4, !tbaa !4
  %4 = bitcast i32* %value.addr.i.i.i.i to i8*
  br label %for.cond.i.i

for.cond.i.i:                                     ; preds = %for.inc.i.i, %entry
  %indvars.iv.i.i = phi i64 [ %indvars.iv.next.i.i, %for.inc.i.i ], [ 0, %entry ]
  %5 = trunc i64 %indvars.iv.i.i to i32
  %cmp.i.i = icmp slt i32 %5, %3
  br i1 %cmp.i.i, label %for.body.i.i, label %_ZN21WorkStealingScheduler11GetNextTaskEi.exit

for.body.i.i:                                     ; preds = %for.cond.i.i
  %fValue.i17.i.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %indvars.iv.i.i, i32 2, i32 0, i32 0
  br label %do.body.i.i.i

do.body.i.i.i:                                    ; preds = %if.else.i.i.i, %for.body.i.i
  %6 = load volatile i32* %fValue.i17.i.i.i, align 4, !tbaa !4
  %7 = trunc i32 %6 to i16
  %8 = lshr i32 %6, 16
  %9 = trunc i32 %8 to i16
  %cmp.i.i.i = icmp eq i16 %7, %9
  br i1 %cmp.i.i.i, label %for.inc.i.i, label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %do.body.i.i.i
  %inc.i.i.i = add i16 %9, 1
  %10 = zext i16 %inc.i.i.i to i32
  %11 = shl nuw i32 %10, 16
  %mask.i.i.i = and i32 %6, 65535
  %ins.i.i.i = or i32 %11, %mask.i.i.i
  call void @llvm.lifetime.start(i64 -1, i8* %4) nounwind
  store volatile i32 %6, i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %12 = load volatile i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %13 = cmpxchg i32* %fValue.i17.i.i.i, i32 %12, i32 %ins.i.i.i seq_cst
  %14 = icmp eq i32 %13, %12
  call void @llvm.lifetime.end(i64 -1, i8* %4) nounwind
  br i1 %14, label %_ZN9TaskQueue7PopTailEv.exit.i.i, label %do.body.i.i.i

_ZN9TaskQueue7PopTailEv.exit.i.i:                 ; preds = %if.else.i.i.i
  %idxprom.i.i.i = sext i16 %9 to i64
  %fTaskList.i.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %indvars.iv.i.i, i32 0
  %15 = load i32** %fTaskList.i.i.i, align 8, !tbaa !6
  %arrayidx.i.i.i = getelementptr inbounds i32* %15, i64 %idxprom.i.i.i
  %16 = load i32* %arrayidx.i.i.i, align 4, !tbaa !4
  %cmp1.i.i = icmp eq i32 %16, 0
  br i1 %cmp1.i.i, label %for.inc.i.i, label %_ZN21WorkStealingScheduler11GetNextTaskEi.exit

for.inc.i.i:                                      ; preds = %do.body.i.i.i, %_ZN9TaskQueue7PopTailEv.exit.i.i
  %indvars.iv.next.i.i = add i64 %indvars.iv.i.i, 1
  br label %for.cond.i.i

_ZN21WorkStealingScheduler11GetNextTaskEi.exit:   ; preds = %for.cond.i.i, %_ZN9TaskQueue7PopTailEv.exit.i.i
  %retval.0.i.i = phi i32 [ %16, %_ZN9TaskQueue7PopTailEv.exit.i.i ], [ 0, %for.cond.i.i ]
  ret i32 %retval.0.i.i
}

define void @initTask(i8* nocapture %scheduler, i32 %task_num, i32 %count) nounwind uwtable ssp {
entry:
  %fTaskGraph.i = getelementptr inbounds i8* %scheduler, i64 16
  %0 = bitcast i8* %fTaskGraph.i to %class.TaskGraph**
  %1 = load %class.TaskGraph** %0, align 8, !tbaa !6
  %idxprom.i.i = sext i32 %task_num to i64
  %fTaskList.i.i = getelementptr inbounds %class.TaskGraph* %1, i64 0, i32 0
  %2 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds i32* %2, i64 %idxprom.i.i
  store volatile i32 %count, i32* %arrayidx.i.i, align 4, !tbaa !4
  ret void
}

define void @activateOutputTask1(i8* nocapture %scheduler, i32 %cur_thread, i32 %task, i32* nocapture %task_num) nounwind uwtable ssp {
entry:
  %fTaskGraph.i = getelementptr inbounds i8* %scheduler, i64 16
  %0 = bitcast i8* %fTaskGraph.i to %class.TaskGraph**
  %1 = load %class.TaskGraph** %0, align 8, !tbaa !6
  %idxprom.i = sext i32 %cur_thread to i64
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %2 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %3 = load %class.TaskQueue** %2, align 8, !tbaa !6
  %idxprom.i.i = sext i32 %task to i64
  %fTaskList.i.i = getelementptr inbounds %class.TaskGraph* %1, i64 0, i32 0
  %4 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds i32* %4, i64 %idxprom.i.i
  %5 = atomicrmw add i32* %arrayidx.i.i, i32 -1 seq_cst
  %cmp.i.i = icmp eq i32 %5, 1
  br i1 %cmp.i.i, label %if.then.i.i, label %_ZN21WorkStealingScheduler18ActivateOutputTaskEiiPi.exit

if.then.i.i:                                      ; preds = %entry
  %6 = load i32* %task_num, align 4, !tbaa !4
  %cmp2.i.i = icmp eq i32 %6, 0
  br i1 %cmp2.i.i, label %if.then3.i.i, label %if.else.i.i

if.then3.i.i:                                     ; preds = %if.then.i.i
  store i32 %task, i32* %task_num, align 4, !tbaa !4
  br label %_ZN21WorkStealingScheduler18ActivateOutputTaskEiiPi.exit

if.else.i.i:                                      ; preds = %if.then.i.i
  %info.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 2, i32 0
  %fHead.i.i.i = bitcast %union.anon* %info.i.i.i to i16*
  %7 = load volatile i16* %fHead.i.i.i, align 2, !tbaa !10
  %idxprom.i.i.i = sext i16 %7 to i64
  %fTaskList.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 0
  %8 = load i32** %fTaskList.i.i.i, align 8, !tbaa !6
  %arrayidx.i.i.i = getelementptr inbounds i32* %8, i64 %idxprom.i.i.i
  store i32 %task, i32* %arrayidx.i.i.i, align 4, !tbaa !4
  %9 = load volatile i16* %fHead.i.i.i, align 2, !tbaa !10
  %inc.i.i.i = add i16 %9, 1
  store volatile i16 %inc.i.i.i, i16* %fHead.i.i.i, align 2, !tbaa !10
  br label %_ZN21WorkStealingScheduler18ActivateOutputTaskEiiPi.exit

_ZN21WorkStealingScheduler18ActivateOutputTaskEiiPi.exit: ; preds = %entry, %if.then3.i.i, %if.else.i.i
  ret void
}

define void @activateOutputTask2(i8* nocapture %scheduler, i32 %cur_thread, i32 %task) nounwind uwtable ssp {
entry:
  %fTaskGraph.i = getelementptr inbounds i8* %scheduler, i64 16
  %0 = bitcast i8* %fTaskGraph.i to %class.TaskGraph**
  %1 = load %class.TaskGraph** %0, align 8, !tbaa !6
  %idxprom.i = sext i32 %cur_thread to i64
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %2 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %3 = load %class.TaskQueue** %2, align 8, !tbaa !6
  %idxprom.i.i = sext i32 %task to i64
  %fTaskList.i.i = getelementptr inbounds %class.TaskGraph* %1, i64 0, i32 0
  %4 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds i32* %4, i64 %idxprom.i.i
  %5 = atomicrmw add i32* %arrayidx.i.i, i32 -1 seq_cst
  %cmp.i.i = icmp eq i32 %5, 1
  br i1 %cmp.i.i, label %if.then.i.i, label %_ZN21WorkStealingScheduler18ActivateOutputTaskEii.exit

if.then.i.i:                                      ; preds = %entry
  %info.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 2, i32 0
  %fHead.i.i.i = bitcast %union.anon* %info.i.i.i to i16*
  %6 = load volatile i16* %fHead.i.i.i, align 2, !tbaa !10
  %idxprom.i.i.i = sext i16 %6 to i64
  %fTaskList.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 0
  %7 = load i32** %fTaskList.i.i.i, align 8, !tbaa !6
  %arrayidx.i.i.i = getelementptr inbounds i32* %7, i64 %idxprom.i.i.i
  store i32 %task, i32* %arrayidx.i.i.i, align 4, !tbaa !4
  %8 = load volatile i16* %fHead.i.i.i, align 2, !tbaa !10
  %inc.i.i.i = add i16 %8, 1
  store volatile i16 %inc.i.i.i, i16* %fHead.i.i.i, align 2, !tbaa !10
  br label %_ZN21WorkStealingScheduler18ActivateOutputTaskEii.exit

_ZN21WorkStealingScheduler18ActivateOutputTaskEii.exit: ; preds = %entry, %if.then.i.i
  ret void
}

define void @activateOneOutputTask(i8* nocapture %scheduler, i32 %cur_thread, i32 %task, i32* nocapture %task_num) nounwind uwtable ssp {
entry:
  %value.addr.i.i.i.i = alloca i32, align 4
  %fTaskGraph.i = getelementptr inbounds i8* %scheduler, i64 16
  %0 = bitcast i8* %fTaskGraph.i to %class.TaskGraph**
  %1 = load %class.TaskGraph** %0, align 8, !tbaa !6
  %idxprom.i = sext i32 %cur_thread to i64
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %2 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %3 = load %class.TaskQueue** %2, align 8, !tbaa !6
  %idxprom.i.i = sext i32 %task to i64
  %fTaskList.i.i = getelementptr inbounds %class.TaskGraph* %1, i64 0, i32 0
  %4 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %arrayidx.i.i = getelementptr inbounds i32* %4, i64 %idxprom.i.i
  %5 = atomicrmw add i32* %arrayidx.i.i, i32 -1 seq_cst
  %cmp.i.i = icmp eq i32 %5, 1
  br i1 %cmp.i.i, label %_ZN21WorkStealingScheduler21ActivateOneOutputTaskEiiPi.exit, label %if.else.i.i

if.else.i.i:                                      ; preds = %entry
  %fValue.i18.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 2, i32 0, i32 0
  %6 = bitcast i32* %value.addr.i.i.i.i to i8*
  br label %do.body.i.i.i

do.body.i.i.i:                                    ; preds = %if.else.i.i.i, %if.else.i.i
  %7 = load volatile i32* %fValue.i18.i.i.i, align 4, !tbaa !4
  %8 = trunc i32 %7 to i16
  %9 = lshr i32 %7, 16
  %10 = trunc i32 %9 to i16
  %cmp.i.i.i = icmp eq i16 %8, %10
  br i1 %cmp.i.i.i, label %_ZN21WorkStealingScheduler21ActivateOneOutputTaskEiiPi.exit, label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %do.body.i.i.i
  %dec.i.i.i = add i16 %8, -1
  %11 = zext i16 %dec.i.i.i to i32
  %mask.i.i.i = and i32 %7, -65536
  %ins.i.i.i = or i32 %11, %mask.i.i.i
  call void @llvm.lifetime.start(i64 -1, i8* %6) nounwind
  store volatile i32 %7, i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %12 = load volatile i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %13 = cmpxchg i32* %fValue.i18.i.i.i, i32 %12, i32 %ins.i.i.i seq_cst
  %14 = icmp eq i32 %13, %12
  call void @llvm.lifetime.end(i64 -1, i8* %6) nounwind
  br i1 %14, label %do.end.i.i.i, label %do.body.i.i.i

do.end.i.i.i:                                     ; preds = %if.else.i.i.i
  %conv17.i.i.i = sext i16 %8 to i64
  %sub.i.i.i = add i64 %conv17.i.i.i, -1
  %fTaskList.i.i.i = getelementptr inbounds %class.TaskQueue* %3, i64 %idxprom.i, i32 0
  %15 = load i32** %fTaskList.i.i.i, align 8, !tbaa !6
  %arrayidx.i.i.i = getelementptr inbounds i32* %15, i64 %sub.i.i.i
  %16 = load i32* %arrayidx.i.i.i, align 4, !tbaa !4
  br label %_ZN21WorkStealingScheduler21ActivateOneOutputTaskEiiPi.exit

_ZN21WorkStealingScheduler21ActivateOneOutputTaskEiiPi.exit: ; preds = %do.body.i.i.i, %entry, %do.end.i.i.i
  %storemerge.i.i = phi i32 [ %task, %entry ], [ %16, %do.end.i.i.i ], [ 0, %do.body.i.i.i ]
  store i32 %storemerge.i.i, i32* %task_num, align 4
  ret void
}

define void @getReadyTask(i8* nocapture %scheduler, i32 %cur_thread, i32* nocapture %task_num) nounwind uwtable ssp {
entry:
  %value.addr.i.i.i.i = alloca i32, align 4
  %idxprom.i = sext i32 %cur_thread to i64
  %fTaskQueueList.i = getelementptr inbounds i8* %scheduler, i64 8
  %0 = bitcast i8* %fTaskQueueList.i to %class.TaskQueue**
  %1 = load %class.TaskQueue** %0, align 8, !tbaa !6
  %2 = load i32* %task_num, align 4, !tbaa !4
  %cmp.i.i = icmp eq i32 %2, 0
  br i1 %cmp.i.i, label %if.then.i.i, label %_ZN21WorkStealingScheduler12GetReadyTaskEiPi.exit

if.then.i.i:                                      ; preds = %entry
  %fValue.i18.i.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %idxprom.i, i32 2, i32 0, i32 0
  %3 = bitcast i32* %value.addr.i.i.i.i to i8*
  br label %do.body.i.i.i

do.body.i.i.i:                                    ; preds = %if.else.i.i.i, %if.then.i.i
  %4 = load volatile i32* %fValue.i18.i.i.i, align 4, !tbaa !4
  %5 = trunc i32 %4 to i16
  %6 = lshr i32 %4, 16
  %7 = trunc i32 %6 to i16
  %cmp.i.i.i = icmp eq i16 %5, %7
  br i1 %cmp.i.i.i, label %_ZN9TaskQueue7PopHeadEv.exit.i.i, label %if.else.i.i.i

if.else.i.i.i:                                    ; preds = %do.body.i.i.i
  %dec.i.i.i = add i16 %5, -1
  %8 = zext i16 %dec.i.i.i to i32
  %mask.i.i.i = and i32 %4, -65536
  %ins.i.i.i = or i32 %8, %mask.i.i.i
  call void @llvm.lifetime.start(i64 -1, i8* %3) nounwind
  store volatile i32 %4, i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %9 = load volatile i32* %value.addr.i.i.i.i, align 4, !tbaa !4
  %10 = cmpxchg i32* %fValue.i18.i.i.i, i32 %9, i32 %ins.i.i.i seq_cst
  %11 = icmp eq i32 %10, %9
  call void @llvm.lifetime.end(i64 -1, i8* %3) nounwind
  br i1 %11, label %do.end.i.i.i, label %do.body.i.i.i

do.end.i.i.i:                                     ; preds = %if.else.i.i.i
  %conv17.i.i.i = sext i16 %5 to i64
  %sub.i.i.i = add i64 %conv17.i.i.i, -1
  %fTaskList.i.i.i = getelementptr inbounds %class.TaskQueue* %1, i64 %idxprom.i, i32 0
  %12 = load i32** %fTaskList.i.i.i, align 8, !tbaa !6
  %arrayidx.i.i.i = getelementptr inbounds i32* %12, i64 %sub.i.i.i
  %13 = load i32* %arrayidx.i.i.i, align 4, !tbaa !4
  br label %_ZN9TaskQueue7PopHeadEv.exit.i.i

_ZN9TaskQueue7PopHeadEv.exit.i.i:                 ; preds = %do.body.i.i.i, %do.end.i.i.i
  %retval.0.i.i.i = phi i32 [ %13, %do.end.i.i.i ], [ 0, %do.body.i.i.i ]
  store i32 %retval.0.i.i.i, i32* %task_num, align 4, !tbaa !4
  br label %_ZN21WorkStealingScheduler12GetReadyTaskEiPi.exit

_ZN21WorkStealingScheduler12GetReadyTaskEiPi.exit: ; preds = %entry, %_ZN9TaskQueue7PopHeadEv.exit.i.i
  ret void
}

define void @initTaskList(i8* nocapture %scheduler, i32 %cur_thread) nounwind uwtable ssp {
entry:
  %0 = bitcast i8* %scheduler to %class.WorkStealingScheduler*
  tail call void @_ZN21WorkStealingScheduler12InitTaskListEi(%class.WorkStealingScheduler* %0, i32 %cur_thread)
  ret void
}

define linkonce_odr void @_ZN21WorkStealingScheduler12InitTaskListEi(%class.WorkStealingScheduler* nocapture %this, i32 %cur_thread) nounwind uwtable ssp align 2 {
entry:
  %fTaskQueueList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 1
  %0 = load %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  %fDynamicNumThreads = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 4
  %1 = load i32* %fDynamicNumThreads, align 4, !tbaa !4
  %cmp1.i = icmp sgt i32 %1, 0
  br i1 %cmp1.i, label %for.body.i, label %_ZN9TaskQueue7InitAllEPS_i.exit

for.body.i:                                       ; preds = %entry, %_ZN9TaskQueue7InitOneEv.exit.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %_ZN9TaskQueue7InitOneEv.exit.i ], [ 0, %entry ]
  %fTaskQueueSize.i.i = getelementptr inbounds %class.TaskQueue* %0, i64 %indvars.iv.i, i32 1
  %2 = load i32* %fTaskQueueSize.i.i, align 4, !tbaa !4
  %cmp2.i.i = icmp sgt i32 %2, 0
  br i1 %cmp2.i.i, label %for.body.lr.ph.i.i, label %_ZN9TaskQueue7InitOneEv.exit.i

for.body.lr.ph.i.i:                               ; preds = %for.body.i
  %fTaskList.i.i = getelementptr inbounds %class.TaskQueue* %0, i64 %indvars.iv.i, i32 0
  %3 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %for.body.lr.ph.i.i
  %indvars.iv.i.i = phi i64 [ 0, %for.body.lr.ph.i.i ], [ %indvars.iv.next.i.i, %for.body.i.i ]
  %arrayidx.i.i = getelementptr inbounds i32* %3, i64 %indvars.iv.i.i
  store i32 -1, i32* %arrayidx.i.i, align 4, !tbaa !4
  %indvars.iv.next.i.i = add i64 %indvars.iv.i.i, 1
  %4 = load i32* %fTaskQueueSize.i.i, align 4, !tbaa !4
  %5 = trunc i64 %indvars.iv.next.i.i to i32
  %cmp.i.i = icmp slt i32 %5, %4
  br i1 %cmp.i.i, label %for.body.i.i, label %_ZN9TaskQueue7InitOneEv.exit.i

_ZN9TaskQueue7InitOneEv.exit.i:                   ; preds = %for.body.i.i, %for.body.i
  %fValue.i.i = getelementptr inbounds %class.TaskQueue* %0, i64 %indvars.iv.i, i32 2, i32 0, i32 0
  store volatile i32 0, i32* %fValue.i.i, align 4, !tbaa !4
  %fStealingStart.i.i = getelementptr inbounds %class.TaskQueue* %0, i64 %indvars.iv.i, i32 3
  store i64 0, i64* %fStealingStart.i.i, align 8, !tbaa !3
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv37 = trunc i64 %indvars.iv.next.i to i32
  %exitcond38 = icmp eq i32 %lftr.wideiv37, %1
  br i1 %exitcond38, label %_ZN9TaskQueue7InitAllEPS_i.exit, label %for.body.i

_ZN9TaskQueue7InitAllEPS_i.exit:                  ; preds = %_ZN9TaskQueue7InitOneEv.exit.i, %entry
  %cmp = icmp eq i32 %cur_thread, -1
  br i1 %cmp, label %for.cond.preheader, label %for.cond7.preheader

for.cond.preheader:                               ; preds = %_ZN9TaskQueue7InitAllEPS_i.exit
  %6 = load i32* %fDynamicNumThreads, align 4, !tbaa !4
  %cmp331 = icmp sgt i32 %6, 0
  br i1 %cmp331, label %for.body.lr.ph, label %if.end

for.body.lr.ph:                                   ; preds = %for.cond.preheader
  %7 = load %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  %fReadyTaskListSize = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 7
  %fReadyTaskList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 6
  %8 = load i32** %fReadyTaskList, align 8, !tbaa !6
  br label %for.body

for.cond7.preheader:                              ; preds = %_ZN9TaskQueue7InitAllEPS_i.exit
  %fReadyTaskListSize8 = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 7
  %9 = load i32* %fReadyTaskListSize8, align 4, !tbaa !4
  %cmp928 = icmp sgt i32 %9, 0
  br i1 %cmp928, label %for.body10.lr.ph, label %if.end

for.body10.lr.ph:                                 ; preds = %for.cond7.preheader
  %idxprom11 = sext i32 %cur_thread to i64
  %10 = load %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  %fReadyTaskList15 = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 6
  %11 = load i32** %fReadyTaskList15, align 8, !tbaa !6
  %info.i = getelementptr inbounds %class.TaskQueue* %10, i64 %idxprom11, i32 2, i32 0
  %fHead.i = bitcast %union.anon* %info.i to i16*
  %fTaskList.i = getelementptr inbounds %class.TaskQueue* %10, i64 %idxprom11, i32 0
  %12 = load i32** %fTaskList.i, align 8, !tbaa !6
  br label %for.body10

for.body:                                         ; preds = %_ZN9TaskQueue12InitTaskListEiPiii.exit, %for.body.lr.ph
  %indvars.iv35 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next36, %_ZN9TaskQueue12InitTaskListEiPiii.exit ]
  %13 = phi i32 [ %6, %for.body.lr.ph ], [ %28, %_ZN9TaskQueue12InitTaskListEiPiii.exit ]
  %arrayidx = getelementptr inbounds %class.TaskQueue* %7, i64 %indvars.iv35
  %14 = load i32* %fReadyTaskListSize, align 4, !tbaa !4
  %div.i = sdiv i32 %14, %13
  %rem.i = srem i32 %14, %13
  %cmp22.i = icmp sgt i32 %div.i, 0
  %15 = trunc i64 %indvars.iv35 to i32
  br i1 %cmp22.i, label %for.body.lr.ph.i, label %for.end.i

for.body.lr.ph.i:                                 ; preds = %for.body
  %mul.i = mul nsw i32 %div.i, %15
  %info.i.i = getelementptr inbounds %class.TaskQueue* %7, i64 %indvars.iv35, i32 2, i32 0
  %fHead.i.i = bitcast %union.anon* %info.i.i to i16*
  %fTaskList.i.i20 = getelementptr inbounds %class.TaskQueue* %arrayidx, i64 0, i32 0
  %16 = load i32** %fTaskList.i.i20, align 8, !tbaa !6
  %17 = sext i32 %mul.i to i64
  br label %for.body.i23

for.body.i23:                                     ; preds = %for.body.i23, %for.body.lr.ph.i
  %indvars.iv25.i = phi i64 [ 0, %for.body.lr.ph.i ], [ %indvars.iv.next26.i, %for.body.i23 ]
  %18 = add nsw i64 %indvars.iv25.i, %17
  %arrayidx.i21 = getelementptr inbounds i32* %8, i64 %18
  %19 = load i32* %arrayidx.i21, align 4, !tbaa !4
  %20 = load volatile i16* %fHead.i.i, align 2, !tbaa !10
  %idxprom.i.i = sext i16 %20 to i64
  %arrayidx.i.i22 = getelementptr inbounds i32* %16, i64 %idxprom.i.i
  store i32 %19, i32* %arrayidx.i.i22, align 4, !tbaa !4
  %21 = load volatile i16* %fHead.i.i, align 2, !tbaa !10
  %inc.i.i = add i16 %21, 1
  store volatile i16 %inc.i.i, i16* %fHead.i.i, align 2, !tbaa !10
  %indvars.iv.next26.i = add i64 %indvars.iv25.i, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next26.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %div.i
  br i1 %exitcond, label %for.end.i, label %for.body.i23

for.end.i:                                        ; preds = %for.body, %for.body.i23
  %cmp2.i = icmp eq i32 %15, 0
  %cmp520.i = icmp sgt i32 %rem.i, 0
  %or.cond.i = and i1 %cmp2.i, %cmp520.i
  br i1 %or.cond.i, label %for.body6.lr.ph.i, label %_ZN9TaskQueue12InitTaskListEiPiii.exit

for.body6.lr.ph.i:                                ; preds = %for.end.i
  %mul7.i = mul nsw i32 %div.i, %13
  %info.i14.i = getelementptr inbounds %class.TaskQueue* %7, i64 %indvars.iv35, i32 2, i32 0
  %fHead.i15.i = bitcast %union.anon* %info.i14.i to i16*
  %fTaskList.i17.i = getelementptr inbounds %class.TaskQueue* %arrayidx, i64 0, i32 0
  %22 = load i32** %fTaskList.i17.i, align 8, !tbaa !6
  %23 = sext i32 %mul7.i to i64
  br label %for.body6.i

for.body6.i:                                      ; preds = %for.body6.i, %for.body6.lr.ph.i
  %indvars.iv.i24 = phi i64 [ 0, %for.body6.lr.ph.i ], [ %indvars.iv.next.i25, %for.body6.i ]
  %24 = add nsw i64 %indvars.iv.i24, %23
  %arrayidx10.i = getelementptr inbounds i32* %8, i64 %24
  %25 = load i32* %arrayidx10.i, align 4, !tbaa !4
  %26 = load volatile i16* %fHead.i15.i, align 2, !tbaa !10
  %idxprom.i16.i = sext i16 %26 to i64
  %arrayidx.i18.i = getelementptr inbounds i32* %22, i64 %idxprom.i16.i
  store i32 %25, i32* %arrayidx.i18.i, align 4, !tbaa !4
  %27 = load volatile i16* %fHead.i15.i, align 2, !tbaa !10
  %inc.i19.i = add i16 %27, 1
  store volatile i16 %inc.i19.i, i16* %fHead.i15.i, align 2, !tbaa !10
  %indvars.iv.next.i25 = add i64 %indvars.iv.i24, 1
  %lftr.wideiv33 = trunc i64 %indvars.iv.next.i25 to i32
  %exitcond34 = icmp eq i32 %lftr.wideiv33, %rem.i
  br i1 %exitcond34, label %_ZN9TaskQueue12InitTaskListEiPiii.exit, label %for.body6.i

_ZN9TaskQueue12InitTaskListEiPiii.exit:           ; preds = %for.body6.i, %for.end.i
  %indvars.iv.next36 = add i64 %indvars.iv35, 1
  %28 = load i32* %fDynamicNumThreads, align 4, !tbaa !4
  %29 = trunc i64 %indvars.iv.next36 to i32
  %cmp3 = icmp slt i32 %29, %28
  br i1 %cmp3, label %for.body, label %if.end

for.body10:                                       ; preds = %for.body10.lr.ph, %for.body10
  %indvars.iv = phi i64 [ 0, %for.body10.lr.ph ], [ %indvars.iv.next, %for.body10 ]
  %arrayidx16 = getelementptr inbounds i32* %11, i64 %indvars.iv
  %30 = load i32* %arrayidx16, align 4, !tbaa !4
  %31 = load volatile i16* %fHead.i, align 2, !tbaa !10
  %idxprom.i = sext i16 %31 to i64
  %arrayidx.i = getelementptr inbounds i32* %12, i64 %idxprom.i
  store i32 %30, i32* %arrayidx.i, align 4, !tbaa !4
  %32 = load volatile i16* %fHead.i, align 2, !tbaa !10
  %inc.i = add i16 %32, 1
  store volatile i16 %inc.i, i16* %fHead.i, align 2, !tbaa !10
  %indvars.iv.next = add i64 %indvars.iv, 1
  %33 = load i32* %fReadyTaskListSize8, align 4, !tbaa !4
  %34 = trunc i64 %indvars.iv.next to i32
  %cmp9 = icmp slt i32 %34, %33
  br i1 %cmp9, label %for.body10, label %if.end

if.end:                                           ; preds = %for.cond.preheader, %_ZN9TaskQueue12InitTaskListEiPiii.exit, %for.cond7.preheader, %for.body10
  ret void
}

define void @addReadyTask(i8* nocapture %scheduler, i32 %task_num) nounwind uwtable ssp {
entry:
  %fReadyTaskListIndex.i = getelementptr inbounds i8* %scheduler, i64 476
  %0 = bitcast i8* %fReadyTaskListIndex.i to i32*
  %1 = load i32* %0, align 4, !tbaa !4
  %inc.i = add nsw i32 %1, 1
  store i32 %inc.i, i32* %0, align 4, !tbaa !4
  %idxprom.i = sext i32 %1 to i64
  %fReadyTaskList.i = getelementptr inbounds i8* %scheduler, i64 464
  %2 = bitcast i8* %fReadyTaskList.i to i32**
  %3 = load i32** %2, align 8, !tbaa !6
  %arrayidx.i = getelementptr inbounds i32* %3, i64 %idxprom.i
  store i32 %task_num, i32* %arrayidx.i, align 4, !tbaa !4
  ret void
}

define linkonce_odr void @_ZN16DynThreadAdapter11StopMeasureEiRi(%class.DynThreadAdapter* nocapture %this, i32 %staticthreadnum, i32* nocapture %dynthreadnum) uwtable inlinehint ssp align 2 {
entry:
  %fDynAdapt = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 6
  %0 = load i8* %fDynAdapt, align 1, !tbaa !8, !range !9
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %return, label %if.end

if.end:                                           ; preds = %entry
  %call.i = tail call i64 @mach_absolute_time()
  %conv.i = uitofp i64 %call.i to double
  %1 = load double* @_ZL10gTimeRatio, align 8, !tbaa !0
  %mul.i = fmul double %conv.i, %1
  %conv1.i = fptoui double %mul.i to i64
  %fStop = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 2
  store i64 %conv1.i, i64* %fStop, align 8, !tbaa !3
  %fCounter = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 3
  %2 = load i32* %fCounter, align 4, !tbaa !4
  %add = add nsw i32 %2, 1
  %rem = srem i32 %add, 50
  store i32 %rem, i32* %fCounter, align 4, !tbaa !4
  %cmp = icmp eq i32 %rem, 0
  br i1 %cmp, label %for.body.i, label %if.end35

for.body.i:                                       ; preds = %if.end, %for.body.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %if.end ]
  %mean.02.i = phi float [ %add.i, %for.body.i ], [ 0.000000e+00, %if.end ]
  %arrayidx.i = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 0, i64 %indvars.iv.i
  %3 = load i64* %arrayidx.i, align 8, !tbaa !3
  %conv.i39 = uitofp i64 %3 to float
  %add.i = fadd float %mean.02.i, %conv.i39
  %indvars.iv.next.i = add i64 %indvars.iv.i, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 50
  br i1 %exitcond, label %_ZN16DynThreadAdapter11ComputeMeanEv.exit, label %for.body.i

_ZN16DynThreadAdapter11ComputeMeanEv.exit:        ; preds = %for.body.i
  %div.i = fdiv float %add.i, 5.000000e+01
  %fOldMean = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 4
  %4 = load float* %fOldMean, align 4, !tbaa !11
  %sub = fsub float %div.i, %4
  %conv = fpext float %sub to double
  %call6 = tail call double @fabs(double %conv)
  %5 = load float* %fOldMean, align 4, !tbaa !11
  %conv8 = fpext float %5 to double
  %div = fdiv double %call6, %conv8
  %cmp9 = fcmp ogt double %div, 0x3FB99999A0000000
  br i1 %cmp9, label %if.then10, label %if.end35

if.then10:                                        ; preds = %_ZN16DynThreadAdapter11ComputeMeanEv.exit
  %cmp12 = fcmp ogt float %div.i, %5
  %fOldfDynamicNumThreads = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 5
  %6 = load i32* %fOldfDynamicNumThreads, align 4, !tbaa !4
  %7 = load i32* %dynthreadnum, align 4, !tbaa !4
  %cmp14 = icmp sgt i32 %6, %7
  store i32 %7, i32* %fOldfDynamicNumThreads, align 4, !tbaa !4
  %8 = load i32* %dynthreadnum, align 4, !tbaa !4
  br i1 %cmp12, label %if.then13, label %if.else21

if.then13:                                        ; preds = %if.then10
  br i1 %cmp14, label %if.then15, label %if.else

if.then15:                                        ; preds = %if.then13
  %add17 = add nsw i32 %8, 1
  store i32 %add17, i32* %dynthreadnum, align 4, !tbaa !4
  br label %if.end31

if.else:                                          ; preds = %if.then13
  %sub19 = add nsw i32 %8, -1
  store i32 %sub19, i32* %dynthreadnum, align 4, !tbaa !4
  br label %if.end31

if.else21:                                        ; preds = %if.then10
  br i1 %cmp14, label %if.then24, label %if.else27

if.then24:                                        ; preds = %if.else21
  %sub26 = add nsw i32 %8, -1
  store i32 %sub26, i32* %dynthreadnum, align 4, !tbaa !4
  br label %if.end31

if.else27:                                        ; preds = %if.else21
  %add29 = add nsw i32 %8, 1
  store i32 %add29, i32* %dynthreadnum, align 4, !tbaa !4
  br label %if.end31

if.end31:                                         ; preds = %if.then24, %if.else27, %if.then15, %if.else
  %9 = phi i32 [ %sub26, %if.then24 ], [ %add29, %if.else27 ], [ %add17, %if.then15 ], [ %sub19, %if.else ]
  store float %div.i, float* %fOldMean, align 4, !tbaa !11
  %cmp.i = icmp slt i32 %9, 1
  br i1 %cmp.i, label %_ZL5Rangeiii.exit, label %if.else.i

if.else.i:                                        ; preds = %if.end31
  %cmp1.i = icmp sgt i32 %9, %staticthreadnum
  %max.val.i = select i1 %cmp1.i, i32 %staticthreadnum, i32 %9
  br label %_ZL5Rangeiii.exit

_ZL5Rangeiii.exit:                                ; preds = %if.end31, %if.else.i
  %retval.0.i = phi i32 [ 1, %if.end31 ], [ %max.val.i, %if.else.i ]
  store i32 %retval.0.i, i32* %dynthreadnum, align 4, !tbaa !4
  br label %if.end35

if.end35:                                         ; preds = %_ZN16DynThreadAdapter11ComputeMeanEv.exit, %_ZL5Rangeiii.exit, %if.end
  %10 = load i64* %fStop, align 8, !tbaa !3
  %fStart = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 1
  %11 = load i64* %fStart, align 8, !tbaa !3
  %sub37 = sub i64 %10, %11
  %12 = load i32* %fCounter, align 4, !tbaa !4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds %class.DynThreadAdapter* %this, i64 0, i32 0, i64 %idxprom
  store i64 %sub37, i64* %arrayidx, align 8, !tbaa !3
  br label %return

return:                                           ; preds = %entry, %if.end35
  ret void
}

declare double @fabs(double)

define linkonce_odr void @_ZN21WorkStealingSchedulerD2Ev(%class.WorkStealingScheduler* nocapture %this) unnamed_addr uwtable ssp align 2 {
entry:
  %fThreadPool = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 0
  %0 = load %class.DSPThreadPool** %fThreadPool, align 8, !tbaa !6
  %isnull = icmp eq %class.DSPThreadPool* %0, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  invoke void @_ZN13DSPThreadPoolD2Ev(%class.DSPThreadPool* %0)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %delete.notnull
  %1 = bitcast %class.DSPThreadPool* %0 to i8*
  tail call void @_ZdlPv(i8* %1) nounwind
  br label %delete.end

delete.end:                                       ; preds = %invoke.cont, %entry
  %fTaskGraph = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 2
  %2 = load %class.TaskGraph** %fTaskGraph, align 8, !tbaa !6
  %isnull2 = icmp eq %class.TaskGraph* %2, null
  br i1 %isnull2, label %delete.end6, label %delete.notnull3

delete.notnull3:                                  ; preds = %delete.end
  %fTaskList.i.i = getelementptr inbounds %class.TaskGraph* %2, i64 0, i32 0
  %3 = load i32** %fTaskList.i.i, align 8, !tbaa !6
  %isnull.i.i = icmp eq i32* %3, null
  br i1 %isnull.i.i, label %invoke.cont5, label %delete.notnull.i.i

delete.notnull.i.i:                               ; preds = %delete.notnull3
  %4 = bitcast i32* %3 to i8*
  tail call void @_ZdaPv(i8* %4) nounwind
  br label %invoke.cont5

invoke.cont5:                                     ; preds = %delete.notnull.i.i, %delete.notnull3
  %5 = bitcast %class.TaskGraph* %2 to i8*
  tail call void @_ZdlPv(i8* %5) nounwind
  br label %delete.end6

delete.end6:                                      ; preds = %invoke.cont5, %delete.end
  %fTaskQueueList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 1
  %6 = load %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  %isnull7 = icmp eq %class.TaskQueue* %6, null
  br i1 %isnull7, label %delete.end20, label %delete.notnull8

delete.notnull8:                                  ; preds = %delete.end6
  %7 = getelementptr inbounds %class.TaskQueue* %6, i64 -1, i32 4
  %8 = bitcast i64* %7 to i8*
  %9 = load i64* %7, align 8
  %arraydestroy.isempty = icmp eq i64 %9, 0
  br i1 %arraydestroy.isempty, label %arraydestroy.done19, label %arraydestroy.body.preheader

arraydestroy.body.preheader:                      ; preds = %delete.notnull8
  %delete.end9 = getelementptr inbounds %class.TaskQueue* %6, i64 %9
  br label %arraydestroy.body

arraydestroy.body:                                ; preds = %arraydestroy.body.preheader, %invoke.cont11
  %arraydestroy.elementPast = phi %class.TaskQueue* [ %arraydestroy.element, %invoke.cont11 ], [ %delete.end9, %arraydestroy.body.preheader ]
  %arraydestroy.element = getelementptr inbounds %class.TaskQueue* %arraydestroy.elementPast, i64 -1
  %fTaskList.i.i25 = getelementptr inbounds %class.TaskQueue* %arraydestroy.element, i64 0, i32 0
  %10 = load i32** %fTaskList.i.i25, align 8, !tbaa !6
  %isnull.i.i26 = icmp eq i32* %10, null
  br i1 %isnull.i.i26, label %invoke.cont11, label %delete.notnull.i.i27

delete.notnull.i.i27:                             ; preds = %arraydestroy.body
  %11 = bitcast i32* %10 to i8*
  tail call void @_ZdaPv(i8* %11) nounwind
  br label %invoke.cont11

invoke.cont11:                                    ; preds = %delete.notnull.i.i27, %arraydestroy.body
  %arraydestroy.done18 = icmp eq %class.TaskQueue* %arraydestroy.element, %6
  br i1 %arraydestroy.done18, label %arraydestroy.done19, label %arraydestroy.body

arraydestroy.done19:                              ; preds = %invoke.cont11, %delete.notnull8
  tail call void @_ZdaPv(i8* %8) nounwind
  br label %delete.end20

delete.end20:                                     ; preds = %arraydestroy.done19, %delete.end6
  %fReadyTaskList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 6
  %12 = load i32** %fReadyTaskList, align 8, !tbaa !6
  %isnull21 = icmp eq i32* %12, null
  br i1 %isnull21, label %delete.end23, label %delete.notnull22

delete.notnull22:                                 ; preds = %delete.end20
  %13 = bitcast i32* %12 to i8*
  tail call void @_ZdaPv(i8* %13) nounwind
  br label %delete.end23

delete.end23:                                     ; preds = %delete.notnull22, %delete.end20
  ret void

lpad:                                             ; preds = %delete.notnull
  %14 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  %15 = bitcast %class.DSPThreadPool* %0 to i8*
  tail call void @_ZdlPv(i8* %15) nounwind
  resume { i8*, i32 } %14
}

define linkonce_odr void @_ZN21WorkStealingSchedulerC2Eii(%class.WorkStealingScheduler* nocapture %this, i32 %task_queue_size, i32 %init_task_list_size) unnamed_addr uwtable ssp align 2 {
entry:
  %physical_count.i = alloca i32, align 4
  %size.i = alloca i64, align 8
  %logical_count.i = alloca i32, align 4
  %fDynThreadAdapter = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 3
  %fCounter.i.i = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 3, i32 3
  store i32 0, i32* %fCounter.i.i, align 4, !tbaa !4
  %fOldMean.i.i = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 3, i32 4
  store float 1.000000e+09, float* %fOldMean.i.i, align 4, !tbaa !11
  %fOldfDynamicNumThreads.i.i = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 3, i32 5
  store i32 1, i32* %fOldfDynamicNumThreads.i.i, align 4, !tbaa !4
  %0 = bitcast %class.DynThreadAdapter* %fDynThreadAdapter to i8*
  call void @llvm.memset.p0i8.i64(i8* %0, i8 0, i64 400, i32 8, i1 false) nounwind
  %call.i.i = call i8* @getenv(i8* getelementptr inbounds ([15 x i8]* @.str5, i64 0, i64 0)) nounwind
  %tobool.i.i = icmp eq i8* %call.i.i, null
  br i1 %tobool.i.i, label %_ZN16DynThreadAdapterC1Ev.exit, label %cond.true.i.i

cond.true.i.i:                                    ; preds = %entry
  %call3.i.i = call i64 @strtol(i8* nocapture %call.i.i, i8** null, i32 10) nounwind
  %phitmp.i.i = icmp ne i64 %call3.i.i, 0
  br label %_ZN16DynThreadAdapterC1Ev.exit

_ZN16DynThreadAdapterC1Ev.exit:                   ; preds = %entry, %cond.true.i.i
  %cond.i.i = phi i1 [ %phitmp.i.i, %cond.true.i.i ], [ false, %entry ]
  %fDynAdapt.i.i = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 3, i32 6
  %frombool.i.i = zext i1 %cond.i.i to i8
  store i8 %frombool.i.i, i8* %fDynAdapt.i.i, align 1, !tbaa !8
  %1 = bitcast i32* %physical_count.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %1)
  %2 = bitcast i64* %size.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %2)
  %3 = bitcast i32* %logical_count.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %3)
  store i32 0, i32* %physical_count.i, align 4, !tbaa !4
  store i64 4, i64* %size.i, align 8, !tbaa !5
  %call.i = call i32 @sysctlbyname(i8* getelementptr inbounds ([15 x i8]* @.str, i64 0, i64 0), i8* %1, i64* %size.i, i8* null, i64 0)
  store i32 0, i32* %logical_count.i, align 4, !tbaa !4
  %call1.i = call i32 @sysctlbyname(i8* getelementptr inbounds ([14 x i8]* @.str1, i64 0, i64 0), i8* %3, i64* %size.i, i8* null, i64 0)
  %4 = load i32* %physical_count.i, align 4, !tbaa !4
  call void @llvm.lifetime.end(i64 -1, i8* %1)
  call void @llvm.lifetime.end(i64 -1, i8* %2)
  call void @llvm.lifetime.end(i64 -1, i8* %3)
  %fStaticNumThreads = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 5
  store i32 %4, i32* %fStaticNumThreads, align 4, !tbaa !4
  %call2 = call i8* @getenv(i8* getelementptr inbounds ([16 x i8]* @.str2, i64 0, i64 0))
  %tobool = icmp eq i8* %call2, null
  br i1 %tobool, label %cond.end, label %cond.true

cond.true:                                        ; preds = %_ZN16DynThreadAdapterC1Ev.exit
  %call4 = call i32 @atoi(i8* %call2)
  br label %cond.end

cond.end:                                         ; preds = %_ZN16DynThreadAdapterC1Ev.exit, %cond.true
  %cond = phi i32 [ %call4, %cond.true ], [ %4, %_ZN16DynThreadAdapterC1Ev.exit ]
  %fDynamicNumThreads = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 4
  store i32 %cond, i32* %fDynamicNumThreads, align 4, !tbaa !4
  %call6 = call noalias i8* @_Znwm(i64 16)
  %5 = bitcast i8* %call6 to %class.DSPThreadPool*
  %6 = sext i32 %4 to i64
  %7 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %6, i64 8)
  %8 = extractvalue { i64, i1 } %7, 1
  %9 = extractvalue { i64, i1 } %7, 0
  %10 = select i1 %8, i64 -1, i64 %9
  %call.i.i2223 = invoke noalias i8* @_Znam(i64 %10)
          to label %call.i.i22.noexc unwind label %lpad

call.i.i22.noexc:                                 ; preds = %cond.end
  %11 = bitcast i8* %call.i.i2223 to %class.DSPThread**
  %fThreadPool.i.i = bitcast i8* %call6 to %class.DSPThread***
  store %class.DSPThread** %11, %class.DSPThread*** %fThreadPool.i.i, align 8, !tbaa !6
  %cmp3.i.i = icmp sgt i32 %4, 0
  br i1 %cmp3.i.i, label %for.body.i.preheader.i, label %invoke.cont

for.body.i.preheader.i:                           ; preds = %call.i.i22.noexc
  store %class.DSPThread* null, %class.DSPThread** %11, align 8, !tbaa !6
  %exitcond.i2.i = icmp eq i32 %4, 1
  br i1 %exitcond.i2.i, label %invoke.cont, label %for.body.for.body_crit_edge.i.i.preheader

for.body.for.body_crit_edge.i.i.preheader:        ; preds = %for.body.i.preheader.i
  %arrayidx.i.i29 = getelementptr inbounds i8* %call.i.i2223, i64 8
  %12 = bitcast i8* %arrayidx.i.i29 to %class.DSPThread**
  store %class.DSPThread* null, %class.DSPThread** %12, align 8, !tbaa !6
  %exitcond.i30 = icmp eq i32 %4, 2
  br i1 %exitcond.i30, label %invoke.cont, label %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i

for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i: ; preds = %for.body.for.body_crit_edge.i.i.preheader, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i.for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i_crit_edge
  %.pre.i.pre.i = phi %class.DSPThread** [ %.pre.i.pre.i.pre, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i.for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i_crit_edge ], [ %11, %for.body.for.body_crit_edge.i.i.preheader ]
  %indvars.iv.next.i.i31 = phi i64 [ %indvars.iv.next.i.i, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i.for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i_crit_edge ], [ 2, %for.body.for.body_crit_edge.i.i.preheader ]
  %arrayidx.i.i = getelementptr inbounds %class.DSPThread** %.pre.i.pre.i, i64 %indvars.iv.next.i.i31
  store %class.DSPThread* null, %class.DSPThread** %arrayidx.i.i, align 8, !tbaa !6
  %indvars.iv.next.i.i = add i64 %indvars.iv.next.i.i31, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next.i.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %4
  br i1 %exitcond, label %invoke.cont, label %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i.for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i_crit_edge

for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i.for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i_crit_edge: ; preds = %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i
  %.pre.i.pre.i.pre = load %class.DSPThread*** %fThreadPool.i.i, align 8, !tbaa !6
  br label %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i

invoke.cont:                                      ; preds = %for.body.for.body_crit_edge.i.i.preheader, %for.body.for.body_crit_edge.i.for.body.for.body_crit_edge.i_crit_edge.i, %for.body.i.preheader.i, %call.i.i22.noexc
  %fThreadCount.i.i = getelementptr inbounds i8* %call6, i64 8
  %13 = bitcast i8* %fThreadCount.i.i to i32*
  store i32 0, i32* %13, align 4, !tbaa !4
  %fCurThreadCount.i.i = getelementptr inbounds i8* %call6, i64 12
  %14 = bitcast i8* %fCurThreadCount.i.i to i32*
  store volatile i32 0, i32* %14, align 4, !tbaa !4
  %fThreadPool = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 0
  store %class.DSPThreadPool* %5, %class.DSPThreadPool** %fThreadPool, align 8, !tbaa !6
  %call8 = call noalias i8* @_Znwm(i64 16)
  %15 = bitcast i8* %call8 to %class.TaskGraph*
  %fTaskQueueSize.i.i = getelementptr inbounds i8* %call8, i64 8
  %16 = bitcast i8* %fTaskQueueSize.i.i to i32*
  store i32 %task_queue_size, i32* %16, align 4, !tbaa !4
  %17 = sext i32 %task_queue_size to i64
  %18 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %17, i64 4)
  %19 = extractvalue { i64, i1 } %18, 1
  %20 = extractvalue { i64, i1 } %18, 0
  %21 = select i1 %19, i64 -1, i64 %20
  %call.i.i2427 = invoke noalias i8* @_Znam(i64 %21)
          to label %call.i.i24.noexc unwind label %lpad9

call.i.i24.noexc:                                 ; preds = %invoke.cont
  %22 = bitcast i8* %call.i.i2427 to i32*
  %fTaskList.i.i = bitcast i8* %call8 to i32**
  store i32* %22, i32** %fTaskList.i.i, align 8, !tbaa !6
  %cmp5.i.i = icmp sgt i32 %task_queue_size, 0
  br i1 %cmp5.i.i, label %for.body.i.i, label %invoke.cont10

for.body.i.i:                                     ; preds = %call.i.i24.noexc, %for.body.i.i
  %indvars.iv.i.i = phi i64 [ %indvars.iv.next.i.i26, %for.body.i.i ], [ 0, %call.i.i24.noexc ]
  %arrayidx.i.i25 = getelementptr inbounds i32* %22, i64 %indvars.iv.i.i
  store volatile i32 0, i32* %arrayidx.i.i25, align 4, !tbaa !4
  %indvars.iv.next.i.i26 = add i64 %indvars.iv.i.i, 1
  %lftr.wideiv34 = trunc i64 %indvars.iv.next.i.i26 to i32
  %exitcond35 = icmp eq i32 %lftr.wideiv34, %task_queue_size
  br i1 %exitcond35, label %invoke.cont10, label %for.body.i.i

invoke.cont10:                                    ; preds = %for.body.i.i, %call.i.i24.noexc
  %fTaskGraph = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 2
  store %class.TaskGraph* %15, %class.TaskGraph** %fTaskGraph, align 8, !tbaa !6
  %23 = load i32* %fStaticNumThreads, align 4, !tbaa !4
  %24 = sext i32 %23 to i64
  %25 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %24, i64 32)
  %26 = extractvalue { i64, i1 } %25, 1
  %27 = extractvalue { i64, i1 } %25, 0
  %28 = call { i64, i1 } @llvm.uadd.with.overflow.i64(i64 %27, i64 8)
  %29 = extractvalue { i64, i1 } %28, 1
  %30 = or i1 %26, %29
  %31 = extractvalue { i64, i1 } %28, 0
  %32 = select i1 %30, i64 -1, i64 %31
  %call12 = call noalias i8* @_Znam(i64 %32)
  %33 = bitcast i8* %call12 to i64*
  store i64 %24, i64* %33, align 8
  %34 = getelementptr inbounds i8* %call12, i64 8
  %35 = bitcast i8* %34 to %class.TaskQueue*
  %isempty = icmp eq i32 %23, 0
  br i1 %isempty, label %arrayctor.cont.thread, label %new.ctorloop

arrayctor.cont.thread:                            ; preds = %invoke.cont10
  %fTaskQueueList38 = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 1
  store %class.TaskQueue* %35, %class.TaskQueue** %fTaskQueueList38, align 8, !tbaa !6
  br label %for.end

new.ctorloop:                                     ; preds = %invoke.cont10
  %arrayctor.end = getelementptr inbounds %class.TaskQueue* %35, i64 %24
  br label %invoke.cont14

invoke.cont14:                                    ; preds = %new.ctorloop, %invoke.cont14
  %arrayctor.cur = phi %class.TaskQueue* [ %35, %new.ctorloop ], [ %arrayctor.next, %invoke.cont14 ]
  %fValue.i.i.i.i = getelementptr inbounds %class.TaskQueue* %arrayctor.cur, i64 0, i32 2, i32 0, i32 0
  store i32 0, i32* %fValue.i.i.i.i, align 4, !tbaa !4
  %arrayctor.next = getelementptr inbounds %class.TaskQueue* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %class.TaskQueue* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %arrayctor.cont, label %invoke.cont14

arrayctor.cont:                                   ; preds = %invoke.cont14
  %.pre = load i32* %fStaticNumThreads, align 4, !tbaa !4
  %fTaskQueueList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 1
  store %class.TaskQueue* %35, %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  %cmp32 = icmp sgt i32 %.pre, 0
  br i1 %cmp32, label %for.body, label %for.end

for.body:                                         ; preds = %arrayctor.cont, %for.body.for.body_crit_edge
  %36 = phi %class.TaskQueue* [ %.pre37, %for.body.for.body_crit_edge ], [ %35, %arrayctor.cont ]
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.for.body_crit_edge ], [ 0, %arrayctor.cont ]
  %arrayidx = getelementptr inbounds %class.TaskQueue* %36, i64 %indvars.iv
  call void @_ZN9TaskQueue4InitEi(%class.TaskQueue* %arrayidx, i32 %task_queue_size)
  %indvars.iv.next = add i64 %indvars.iv, 1
  %37 = load i32* %fStaticNumThreads, align 4, !tbaa !4
  %38 = trunc i64 %indvars.iv.next to i32
  %cmp = icmp slt i32 %38, %37
  br i1 %cmp, label %for.body.for.body_crit_edge, label %for.end

for.body.for.body_crit_edge:                      ; preds = %for.body
  %.pre37 = load %class.TaskQueue** %fTaskQueueList, align 8, !tbaa !6
  br label %for.body

lpad:                                             ; preds = %cond.end
  %39 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  %40 = extractvalue { i8*, i32 } %39, 0
  %41 = extractvalue { i8*, i32 } %39, 1
  call void @_ZdlPv(i8* %call6) nounwind
  br label %eh.resume

lpad9:                                            ; preds = %invoke.cont
  %42 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  %43 = extractvalue { i8*, i32 } %42, 0
  %44 = extractvalue { i8*, i32 } %42, 1
  call void @_ZdlPv(i8* %call8) nounwind
  br label %eh.resume

for.end:                                          ; preds = %for.body, %arrayctor.cont.thread, %arrayctor.cont
  %fReadyTaskListSize = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 7
  store i32 %init_task_list_size, i32* %fReadyTaskListSize, align 4, !tbaa !4
  %45 = sext i32 %init_task_list_size to i64
  %46 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %45, i64 4)
  %47 = extractvalue { i64, i1 } %46, 1
  %48 = extractvalue { i64, i1 } %46, 0
  %49 = select i1 %47, i64 -1, i64 %48
  %call20 = call noalias i8* @_Znam(i64 %49)
  %50 = bitcast i8* %call20 to i32*
  %fReadyTaskList = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 6
  store i32* %50, i32** %fReadyTaskList, align 8, !tbaa !6
  %fReadyTaskListIndex = getelementptr inbounds %class.WorkStealingScheduler* %this, i64 0, i32 8
  store i32 0, i32* %fReadyTaskListIndex, align 4, !tbaa !4
  ret void

eh.resume:                                        ; preds = %lpad9, %lpad
  %exn.slot.0 = phi i8* [ %43, %lpad9 ], [ %40, %lpad ]
  %ehselector.slot.0 = phi i32 [ %44, %lpad9 ], [ %41, %lpad ]
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.0, 0
  %lpad.val21 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.0, 1
  resume { i8*, i32 } %lpad.val21
}

declare i8* @getenv(i8* nocapture) nounwind readonly

declare i32 @atoi(i8* nocapture) nounwind readonly

declare { i64, i1 } @llvm.uadd.with.overflow.i64(i64, i64) nounwind readnone

define linkonce_odr void @_ZN9TaskQueue4InitEi(%class.TaskQueue* nocapture %this, i32 %task_queue_size) uwtable inlinehint ssp align 2 {
entry:
  %fTaskQueueSize = getelementptr inbounds %class.TaskQueue* %this, i64 0, i32 1
  store i32 %task_queue_size, i32* %fTaskQueueSize, align 4, !tbaa !4
  %0 = sext i32 %task_queue_size to i64
  %1 = tail call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %0, i64 4)
  %2 = extractvalue { i64, i1 } %1, 1
  %3 = extractvalue { i64, i1 } %1, 0
  %4 = select i1 %2, i64 -1, i64 %3
  %call = tail call noalias i8* @_Znam(i64 %4)
  %5 = bitcast i8* %call to i32*
  %fTaskList = getelementptr inbounds %class.TaskQueue* %this, i64 0, i32 0
  store i32* %5, i32** %fTaskList, align 8, !tbaa !6
  %cmp20 = icmp sgt i32 %task_queue_size, 0
  br i1 %cmp20, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32* %5, i64 %indvars.iv
  store i32 -1, i32* %arrayidx, align 4, !tbaa !4
  %indvars.iv.next = add i64 %indvars.iv, 1
  %6 = trunc i64 %indvars.iv.next to i32
  %cmp = icmp slt i32 %6, %task_queue_size
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body, %entry
  %fStealingStart = getelementptr inbounds %class.TaskQueue* %this, i64 0, i32 3
  store i64 0, i64* %fStealingStart, align 8, !tbaa !3
  %call5 = tail call i8* @getenv(i8* getelementptr inbounds ([13 x i8]* @.str3, i64 0, i64 0))
  %tobool = icmp eq i8* %call5, null
  br i1 %tobool, label %cond.end, label %cond.true

cond.true:                                        ; preds = %for.end
  %call7 = tail call i64 @strtoll(i8* nocapture %call5, i8** null, i32 10)
  %phitmp = sdiv i64 %call7, 1000000
  %phitmp19 = trunc i64 %phitmp to i32
  br label %cond.end

cond.end:                                         ; preds = %for.end, %cond.true
  %cond = phi i32 [ %phitmp19, %cond.true ], [ 2500, %for.end ]
  %call8 = tail call i8* @getenv(i8* getelementptr inbounds ([17 x i8]* @.str4, i64 0, i64 0))
  %tobool9 = icmp eq i8* %call8, null
  br i1 %tobool9, label %cond.false14, label %cond.true10

cond.true10:                                      ; preds = %cond.end
  %call12 = tail call i64 @strtoll(i8* nocapture %call8, i8** null, i32 10)
  %conv13 = sext i32 %cond to i64
  %mul = mul nsw i64 %call12, %conv13
  br label %cond.end17

cond.false14:                                     ; preds = %cond.end
  %mul15 = mul nsw i32 %cond, 50
  %conv16 = sext i32 %mul15 to i64
  br label %cond.end17

cond.end17:                                       ; preds = %cond.false14, %cond.true10
  %cond18 = phi i64 [ %mul, %cond.true10 ], [ %conv16, %cond.false14 ]
  %fMaxStealing = getelementptr inbounds %class.TaskQueue* %this, i64 0, i32 4
  store i64 %cond18, i64* %fMaxStealing, align 8, !tbaa !3
  ret void
}

declare i64 @strtoll(i8*, i8** nocapture, i32) nounwind

declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) nounwind

declare i64 @strtol(i8*, i8** nocapture, i32) nounwind

declare i32 @semaphore_signal(i32)

declare i32 @pthread_mach_thread_np(%struct._opaque_pthread_t*)

declare i32 @thread_terminate(i32)

declare i32 @pthread_attr_init(%struct._opaque_pthread_attr_t*)

declare i32 @pthread_attr_setdetachstate(%struct._opaque_pthread_attr_t*, i32)

declare i32 @printf(i8* nocapture, ...) nounwind

declare i8* @"\01_strerror"(i32)

declare i32* @__error()

declare i32 @pthread_attr_setscope(%struct._opaque_pthread_attr_t*, i32)

declare i32 @pthread_attr_setinheritsched(%struct._opaque_pthread_attr_t*, i32)

declare i32 @pthread_attr_setschedpolicy(%struct._opaque_pthread_attr_t*, i32)

declare i32 @pthread_attr_setschedparam(%struct._opaque_pthread_attr_t*, %struct.sched_param*)

declare i32 @pthread_attr_setstacksize(%struct._opaque_pthread_attr_t*, i64)

declare i32 @pthread_create(%struct._opaque_pthread_t**, %struct._opaque_pthread_attr_t*, i8* (i8*)*, i8*)

define linkonce_odr noalias i8* @_ZN9DSPThread13ThreadHandlerEPv(i8* nocapture %arg) noreturn uwtable ssp align 2 {
entry:
  %theTCPolicy.i.i = alloca %struct.thread_time_constraint_policy, align 4
  %theTCPolicy.i = alloca i32, align 4
  %count.i = alloca i32, align 4
  %get_default.i = alloca i32, align 4
  %0 = alloca i32, align 4
  %1 = alloca i32, align 4
  %2 = bitcast i32* %1 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %2) nounwind
  call void @llvm.x86.sse.stmxcsr(i8* %2) nounwind
  %stmxcsr.i = load i32* %1, align 4
  call void @llvm.lifetime.end(i64 -1, i8* %2) nounwind
  %or = or i32 %stmxcsr.i, 32832
  %3 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %3) nounwind
  store i32 %or, i32* %0, align 4
  call void @llvm.x86.sse.ldmxcsr(i8* %3) nounwind
  call void @llvm.lifetime.end(i64 -1, i8* %3) nounwind
  %fThread = getelementptr inbounds i8* %arg, i64 8
  %4 = bitcast i8* %fThread to %struct._opaque_pthread_t**
  %5 = load %struct._opaque_pthread_t** %4, align 8, !tbaa !6
  %6 = bitcast i32* %theTCPolicy.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %6)
  %7 = bitcast i32* %count.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %7)
  %8 = bitcast i32* %get_default.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %8)
  store i32 1, i32* %count.i, align 4, !tbaa !4
  store i32 0, i32* %get_default.i, align 4, !tbaa !4
  %call.i = call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %5)
  %call1.i = call i32 @thread_policy_get(i32 %call.i, i32 4, i32* %theTCPolicy.i, i32* %count.i, i32* %get_default.i)
  call void @llvm.lifetime.end(i64 -1, i8* %6)
  call void @llvm.lifetime.end(i64 -1, i8* %7)
  call void @llvm.lifetime.end(i64 -1, i8* %8)
  %9 = getelementptr inbounds i8* %arg, i64 28
  %10 = load i8* %9, align 1, !tbaa !8, !range !9
  %tobool = icmp eq i8 %10, 0
  %_sem.i.i.pre = getelementptr inbounds i8* %arg, i64 24
  %.pre = bitcast i8* %_sem.i.i.pre to i32*
  br i1 %tobool, label %entry.while.body.preheader_crit_edge, label %if.then

entry.while.body.preheader_crit_edge:             ; preds = %entry
  %fDSP.i.pre = getelementptr inbounds i8* %arg, i64 40
  %.pre8 = bitcast i8* %fDSP.i.pre to i8**
  %fNumThread.i.pre = getelementptr inbounds i8* %arg, i64 32
  %.pre10 = bitcast i8* %fNumThread.i.pre to i32*
  br label %while.body.preheader

if.then:                                          ; preds = %entry
  %11 = load i32* %.pre, align 4, !tbaa !4
  %call.i.i2 = call i32 @semaphore_wait(i32 %11)
  %fDSP.i3 = getelementptr inbounds i8* %arg, i64 40
  %12 = bitcast i8* %fDSP.i3 to i8**
  %13 = load i8** %12, align 8, !tbaa !6
  %fNumThread.i4 = getelementptr inbounds i8* %arg, i64 32
  %14 = bitcast i8* %fNumThread.i4 to i32*
  %15 = load i32* %14, align 4, !tbaa !4
  %add.i5 = add nsw i32 %15, 1
  call void @computeThreadExternal(i8* %13, i32 %add.i5)
  %call.i6 = call %struct._opaque_pthread_t* @pthread_self()
  %16 = load i64* @_ZL7gPeriod, align 8, !tbaa !3
  %17 = load i64* @_ZL12gComputation, align 8, !tbaa !3
  %18 = load i64* @_ZL11gConstraint, align 8, !tbaa !3
  %19 = bitcast %struct.thread_time_constraint_policy* %theTCPolicy.i.i to i8*
  call void @llvm.lifetime.start(i64 -1, i8* %19)
  %conv.i.i = trunc i64 %16 to i32
  %period1.i.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i.i, i64 0, i32 0
  store i32 %conv.i.i, i32* %period1.i.i, align 4, !tbaa !4
  %conv2.i.i = trunc i64 %17 to i32
  %computation3.i.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i.i, i64 0, i32 1
  store i32 %conv2.i.i, i32* %computation3.i.i, align 4, !tbaa !4
  %conv4.i.i = trunc i64 %18 to i32
  %constraint5.i.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i.i, i64 0, i32 2
  store i32 %conv4.i.i, i32* %constraint5.i.i, align 4, !tbaa !4
  %preemptible.i.i = getelementptr inbounds %struct.thread_time_constraint_policy* %theTCPolicy.i.i, i64 0, i32 3
  store i32 1, i32* %preemptible.i.i, align 4, !tbaa !4
  %call.i.i7 = call i32 @pthread_mach_thread_np(%struct._opaque_pthread_t* %call.i6)
  %call6.i.i = call i32 @thread_policy_set(i32 %call.i.i7, i32 2, i32* %period1.i.i, i32 4)
  call void @llvm.lifetime.end(i64 -1, i8* %19)
  br label %while.body.preheader

while.body.preheader:                             ; preds = %entry.while.body.preheader_crit_edge, %if.then
  %.pre-phi11 = phi i32* [ %.pre10, %entry.while.body.preheader_crit_edge ], [ %14, %if.then ]
  %.pre-phi9 = phi i8** [ %.pre8, %entry.while.body.preheader_crit_edge ], [ %12, %if.then ]
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %20 = load i32* %.pre, align 4, !tbaa !4
  %call.i.i = call i32 @semaphore_wait(i32 %20)
  %21 = load i8** %.pre-phi9, align 8, !tbaa !6
  %22 = load i32* %.pre-phi11, align 4, !tbaa !4
  %add.i = add nsw i32 %22, 1
  call void @computeThreadExternal(i8* %21, i32 %add.i)
  br label %while.body
}

declare i32 @pthread_attr_destroy(%struct._opaque_pthread_attr_t*)

declare i32 @thread_policy_set(i32, i32, i32*, i32)

declare extern_weak void @computeThreadExternal(i8*, i32)

declare i32 @semaphore_wait(i32)

declare i32 @thread_policy_get(i32, i32, i32*, i32*, i32*)

declare void @llvm.x86.sse.stmxcsr(i8*) nounwind

declare void @llvm.x86.sse.ldmxcsr(i8*) nounwind

define linkonce_odr void @_ZN9DSPThreadD1Ev(%class.DSPThread* nocapture %this) unnamed_addr uwtable ssp align 2 {
entry:
  %0 = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ([4 x i8*]* @_ZTV9DSPThread, i64 0, i64 2) to i32 (...)**), i32 (...)*** %0, align 8, !tbaa !7
  %1 = load i32* @mach_task_self_, align 4, !tbaa !4
  %_sem.i.i.i = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 3, i32 0
  %2 = load i32* %_sem.i.i.i, align 4, !tbaa !4
  %call.i.i.i = tail call i32 @semaphore_destroy(i32 %1, i32 %2)
  ret void
}

define linkonce_odr void @_ZN9DSPThreadD0Ev(%class.DSPThread* %this) unnamed_addr uwtable ssp align 2 {
entry:
  %0 = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ([4 x i8*]* @_ZTV9DSPThread, i64 0, i64 2) to i32 (...)**), i32 (...)*** %0, align 8, !tbaa !7
  %1 = load i32* @mach_task_self_, align 4, !tbaa !4
  %_sem.i.i.i.i = getelementptr inbounds %class.DSPThread* %this, i64 0, i32 3, i32 0
  %2 = load i32* %_sem.i.i.i.i, align 4, !tbaa !4
  %call.i.i.i.i3 = invoke i32 @semaphore_destroy(i32 %1, i32 %2)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  %3 = bitcast %class.DSPThread* %this to i8*
  tail call void @_ZdlPv(i8* %3) nounwind
  ret void

lpad:                                             ; preds = %entry
  %4 = landingpad { i8*, i32 } personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*)
          cleanup
  %5 = bitcast %class.DSPThread* %this to i8*
  tail call void @_ZdlPv(i8* %5) nounwind
  resume { i8*, i32 } %4
}

declare i32 @semaphore_destroy(i32, i32)

declare i32 @semaphore_create(i32, i32*, i32, i32)

declare i8* @__cxa_allocate_exception(i64)

declare void @__cxa_throw(i8*, i8*, i8*)

declare i32 @mach_timebase_info(%struct.mach_timebase_info*)

declare void @llvm.lifetime.start(i64, i8* nocapture) nounwind

declare void @llvm.lifetime.end(i64, i8* nocapture) nounwind

!0 = metadata !{metadata !"double", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
!3 = metadata !{metadata !"long long", metadata !1}
!4 = metadata !{metadata !"int", metadata !1}
!5 = metadata !{metadata !"long", metadata !1}
!6 = metadata !{metadata !"any pointer", metadata !1}
!7 = metadata !{metadata !"vtable pointer", metadata !2}
!8 = metadata !{metadata !"bool", metadata !1}
!9 = metadata !{i8 0, i8 2}                       
!10 = metadata !{metadata !"short", metadata !1}
!11 = metadata !{metadata !"float", metadata !1}
