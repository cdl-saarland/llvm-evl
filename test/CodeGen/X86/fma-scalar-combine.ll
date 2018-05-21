; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu -mattr=+avx512f -mattr=+fma -show-mc-encoding | FileCheck %s --check-prefix=CHECK --check-prefix=SKX

define <2 x double> @combine_scalar_mask_fmadd_f32(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fmadd_f32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fadd fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %3
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask_fmadd_f64(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fmadd_f64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fadd fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %0
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_maskz_fmadd_32(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fmadd_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0x7e,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fadd fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float 0.000000e+00
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_maskz_fmadd_64(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fmadd_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0xff,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fadd fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double 0.000000e+00
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask3_fmadd_32(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fmadd_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovaps %xmm2, %xmm0 # encoding: [0xc5,0xf8,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fadd fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %5
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask3_fmadd_64(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fmadd_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xa9,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovapd %xmm2, %xmm0 # encoding: [0xc5,0xf9,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fadd fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %2
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask_fmsub_f32(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fmsub_f32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %3
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask_fmsub_f64(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fmsub_f64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %0
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_maskz_fmsub_32(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fmsub_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0x7e,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float 0.000000e+00
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_maskz_fmsub_64(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fmsub_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0xff,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double 0.000000e+00
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask3_fmsub_32(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fmsub_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovaps %xmm2, %xmm0 # encoding: [0xc5,0xf8,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %6, %5
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %5
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask3_fmsub_64(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fmsub_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xab,0xca]
; CHECK-NEXT:    # xmm1 = (xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovapd %xmm2, %xmm0 # encoding: [0xc5,0xf9,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %3, %2
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %2
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask_fnmadd_f32(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fnmadd_f32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %5, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %3
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask_fnmadd_f64(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fnmadd_f64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %2, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %0
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_maskz_fnmadd_32(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fnmadd_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0x7e,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %5, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float 0.000000e+00
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_maskz_fnmadd_64(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fnmadd_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0xff,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %2, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double 0.000000e+00
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask3_fnmadd_32(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fnmadd_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovaps %xmm2, %xmm0 # encoding: [0xc5,0xf8,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %5, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %5
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask3_fnmadd_64(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fnmadd_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmadd213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xad,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) + xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovapd %xmm2, %xmm0 # encoding: [0xc5,0xf9,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %2, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %2
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask_fnmsub_f32(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fnmsub_f32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %sub = fsub fast float -0.000000e+00, %5
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %sub, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %3
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask_fnmsub_f64(<2 x double> %a, i8 zeroext %k, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_mask_fnmsub_f64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %sub = fsub fast double -0.000000e+00, %2
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %sub, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %0
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_maskz_fnmsub_32(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fnmsub_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0x7e,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %sub = fsub fast float -0.000000e+00, %5
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %sub, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float 0.000000e+00
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_maskz_fnmsub_64(i8 zeroext %k, <2 x double> %a, <2 x double> %b, <2 x double> %c) {
; CHECK-LABEL: combine_scalar_maskz_fnmsub_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm0 {%k1} {z} # encoding: [0x62,0xf1,0xff,0x89,0x10,0xc1]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %sub = fsub fast double -0.000000e+00, %2
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %sub, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double 0.000000e+00
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}

define <2 x double> @combine_scalar_mask3_fnmsub_32(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fnmsub_32:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213ss %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0x79,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovss %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0x7e,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovaps %xmm2, %xmm0 # encoding: [0xc5,0xf8,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = bitcast <2 x double> %a to <4 x float>
  %1 = bitcast <2 x double> %b to <4 x float>
  %2 = bitcast <2 x double> %c to <4 x float>
  %3 = extractelement <4 x float> %0, i64 0
  %4 = extractelement <4 x float> %1, i64 0
  %5 = extractelement <4 x float> %2, i64 0
  %sub = fsub fast float -0.000000e+00, %5
  %6 = fmul fast float %4, %3
  %7 = fsub fast float %sub, %6
  %8 = bitcast i8 %k to <8 x i1>
  %9 = extractelement <8 x i1> %8, i64 0
  %10 = select i1 %9, float %7, float %5
  %11 = insertelement <4 x float> %0, float %10, i64 0
  %12 = bitcast <4 x float> %11 to <2 x double>
  ret <2 x double> %12
}

define <2 x double> @combine_scalar_mask3_fnmsub_64(<2 x double> %a, <2 x double> %b, <2 x double> %c, i8 zeroext %k) {
; CHECK-LABEL: combine_scalar_mask3_fnmsub_64:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vfnmsub213sd %xmm2, %xmm0, %xmm1 # EVEX TO VEX Compression encoding: [0xc4,0xe2,0xf9,0xaf,0xca]
; CHECK-NEXT:    # xmm1 = -(xmm0 * xmm1) - xmm2
; CHECK-NEXT:    kmovw %edi, %k1 # encoding: [0xc5,0xf8,0x92,0xcf]
; CHECK-NEXT:    vmovsd %xmm1, %xmm0, %xmm2 {%k1} # encoding: [0x62,0xf1,0xff,0x09,0x10,0xd1]
; CHECK-NEXT:    vmovapd %xmm2, %xmm0 # encoding: [0xc5,0xf9,0x28,0xc2]
; CHECK-NEXT:    retq # encoding: [0xc3]
entry:
  %0 = extractelement <2 x double> %a, i64 0
  %1 = extractelement <2 x double> %b, i64 0
  %2 = extractelement <2 x double> %c, i64 0
  %sub = fsub fast double -0.000000e+00, %2
  %3 = fmul fast double %1, %0
  %4 = fsub fast double %sub, %3
  %5 = bitcast i8 %k to <8 x i1>
  %6 = extractelement <8 x i1> %5, i64 0
  %7 = select i1 %6, double %4, double %2
  %8 = insertelement <2 x double> %a, double %7, i64 0
  ret <2 x double> %8
}
