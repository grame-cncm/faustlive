; ModuleID = 'llvm_math.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.8.0"

define float @fabsf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @fabs(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @fabs(double)

define float @acosf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @acos(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @acos(double)

define float @asinf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @asin(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @asin(double)

define float @atanf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @atan(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @atan(double)

define float @atan2f(float %x, float %y) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %conv1 = fpext float %y to double
  %call = tail call double @atan2(double %conv, double %conv1) nounwind
  %conv2 = fptrunc double %call to float
  ret float %conv2
}

declare double @atan2(double, double)

define float @ceilf(float %x) nounwind uwtable ssp {
entry:
  br label %tailrecurse

tailrecurse:                                      ; preds = %tailrecurse, %entry
  br label %tailrecurse
}

define float @cosf(float %x) nounwind uwtable readnone ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @cos(double %conv) nounwind readnone
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @cos(double) nounwind readnone

define float @coshf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @cosh(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @cosh(double)

define float @expf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @exp(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @exp(double)

define float @floorf(float %x) nounwind uwtable ssp {
entry:
  %floorf = tail call float @floorf(float %x)
  ret float %floorf
}

define float @fmodf(float %x, float %y) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %conv1 = fpext float %y to double
  %call = tail call double @fmod(double %conv, double %conv1) nounwind
  %conv2 = fptrunc double %call to float
  ret float %conv2
}

declare double @fmod(double, double)

define float @logf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @log(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @log(double)

define float @log10f(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @log10(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @log10(double)

define float @powf(float %x, float %y) nounwind uwtable readnone ssp {
entry:
  %conv = fpext float %x to double
  %conv1 = fpext float %y to double
  %0 = tail call double @llvm.pow.f64(double %conv, double %conv1)
  %conv2 = fptrunc double %0 to float
  ret float %conv2
}

declare double @llvm.pow.f64(double, double) nounwind readonly

define float @roundf(float %x) nounwind uwtable ssp {
entry:
  br label %tailrecurse

tailrecurse:                                      ; preds = %tailrecurse, %entry
  br label %tailrecurse
}

define float @sinf(float %x) nounwind uwtable readnone ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @sin(double %conv) nounwind readnone
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @sin(double) nounwind readnone

define float @sinhf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @sinh(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @sinh(double)

define float @sqrtf(float %x) nounwind uwtable readnone ssp {
entry:
  %conv1 = tail call float @sqrtf(float %x) nounwind readnone
  ret float %conv1
}

define float @tanf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @tan(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @tan(double)

define float @tanhf(float %x) nounwind uwtable ssp {
entry:
  %conv = fpext float %x to double
  %call = tail call double @tanh(double %conv) nounwind
  %conv1 = fptrunc double %call to float
  ret float %conv1
}

declare double @tanh(double)
