; RUN: opt < %s -instsimplify -S | FileCheck %s

define <8 x double> @fsub_fadd_fold_vp_xy(<8 x double> %x, <8 x double> %y, <8 x i1> %m, i32 %len) {
; CHECK-LABEL: fsub_fadd_fold_vp_xy
;  CHECK:   ret <8 x double> %x
  %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %x, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  %res0 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  ret <8 x double> %res0
}

define <8 x double> @fsub_fadd_fold_vp_zw(<8 x double> %z, <8 x double> %w, <8 x i1> %m, i32 %len) {
; CHECK-LABEL: fsub_fadd_fold_vp_zw
;  CHECK:   ret <8 x double> %z
  %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %w, <8 x double> %z, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  %res1 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %w, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  ret <8 x double> %res1
}

define <8 x double> @fsub_fadd_fold_vp_yx_fpexcept(<8 x double> %x, <8 x double> %y, <8 x i1> %m, i32 %len) #0 {
; CHECK-LABEL: fsub_fadd_fold_vp_yx
;  CHECK-NEXT:   %tmp =
;  CHECK-NEXT:   %res2 =
;  CHECK-NEXT:   ret
  %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %y, <8 x double> %x, metadata !"round.tonearest", metadata !"fpexcept.strict", <8 x i1> %m, i32 %len)
  %res2 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.strict", <8 x i1> %m, i32 %len)
  ret <8 x double> %res2
}

define <8 x double> @fsub_fadd_fold_vp_yx_olen(<8 x double> %x, <8 x double> %y, <8 x i1> %m, i32 %len, i32 %otherLen) {
; CHECK-LABEL: fsub_fadd_fold_vp_yx_olen
;  CHECK-NEXT:   %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %y, <8 x double> %x, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %otherLen)
;  CHECK-NEXT:   %res3 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
;  CHECK-NEXT:   ret <8 x double> %res3
  %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %y, <8 x double> %x, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %otherLen)
  %res3 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  ret <8 x double> %res3
}

define <8 x double> @fsub_fadd_fold_vp_yx_omask(<8 x double> %x, <8 x double> %y, <8 x i1> %m, i32 %len, <8 x i1> %othermask) {
; CHECK-LABEL: fsub_fadd_fold_vp_yx_omask
;  CHECK-NEXT:   %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %y, <8 x double> %x, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
;  CHECK-NEXT:   %res4 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %othermask, i32 %len)
;  CHECK-NEXT:   ret <8 x double> %res4
  %tmp = call reassoc nsz <8 x double> @llvm.vp.fadd.v8f64(<8 x double> %y, <8 x double> %x, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %m, i32 %len)
  %res4 = call reassoc nsz <8 x double> @llvm.vp.fsub.v8f64(<8 x double> %tmp, <8 x double> %y, metadata !"round.tonearest", metadata !"fpexcept.ignore", <8 x i1> %othermask, i32 %len)
  ret <8 x double> %res4
}

; Function Attrs: nounwind readnone
declare <8 x double> @llvm.vp.fadd.v8f64(<8 x double>, <8 x double>, metadata, metadata, <8 x i1> mask, i32 vlen)

; Function Attrs: nounwind readnone
declare <8 x double> @llvm.vp.fsub.v8f64(<8 x double>, <8 x double>, metadata, metadata, <8 x i1> mask, i32 vlen)

attributes #0 = { strictfp }
