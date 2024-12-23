// Copyright 2024 Mickael Daniel. All Rights Reserved.

#pragma once

#include <stdio.h>
#include "tree_sitter/api.h"

// Fixup all the unresolved symbols linker errors
//
// Note: This must have to do with how I compile the static library for tree-sitter (using zig).
extern "C" {
	
void __stack_chk_fail(void)
{
}

void __stack_chk_guard(void)
{
}

int __mingw_vfprintf(_Inout_ FILE* const InStream, _In_z_ _Printf_format_string_ char const* const InFormat, ...)
{
	va_list ArgList;
	__crt_va_start(ArgList, InFormat);
	const int Result = _vfprintf_l(InStream, InFormat, NULL, ArgList);
	__crt_va_end(ArgList);
	return Result;
}

int __mingw_vsnprintf(_Out_writes_opt_(_BufferCount) _Always_(_Post_z_) char* const InBuffer, _In_ size_t const InBufferCount, _In_z_ _Printf_format_string_ char const* const InFormat, ...)
{
	va_list ArgList;
	__crt_va_start(ArgList, InFormat);
	const int Result = snprintf(InBuffer, InBufferCount, InFormat, ArgList);
	__crt_va_end(ArgList);
	return Result;
}
	
}

