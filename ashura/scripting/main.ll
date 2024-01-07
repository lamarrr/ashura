; ModuleID = 'main.cc'
source_filename = "main.cc"
target datalayout = "e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-n32:64-S128-ni:1:10:20"
target triple = "wasm32"

%struct.Module = type { ptr, ptr, ptr }

@shader0_module = hidden local_unnamed_addr constant %struct.Module { ptr @shader0_init, ptr @shader0_deinit, ptr @shader0_tick }, align 4

declare void @shader0_init(ptr noundef) #0

declare void @shader0_deinit(ptr noundef) #0

declare void @shader0_tick(ptr noundef) #0

attributes #0 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+mutable-globals,+sign-ext" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 17.0.6"}
