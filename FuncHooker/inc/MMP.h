/************************************************************************************\
 * FuncHooker - An Andrew Shurney Production                                        *
\************************************************************************************/

/*! \file		MMP.h
 *  \author		Andrew Shurney
 *  \brief		Macro meta-programming library
 */

#ifndef MMP_H
#define MMP_H

/*! \brief Defines a comma as a token without interfering with the preprocessor */
#define MMP_COMMA                               MMP_UNPAR(,)
#define MMP_LEFT_PAREN                          (
#define MMP_RIGHT_PAREN                         )

#define MMP_CONCAT(X, Y)                        X##Y
/*! \breif enumerates "X" from 1 to num with "INTER" as separator

	MMP_ENUM(3,class T, = void MMP_COMMA) --> class T1 = void, class T2 = void, class T3 = void
*/
#define MMP_ENUM(num, X, INTER)                  MMP_ENUM_HELPER(num, X, MMP_UNPAR(INTER))
#define MMP_ENUM_HELPER(num, X, INTER)           MMP_ENUM_ ## num (X, MMP_UNPAR(INTER))

/*! \brief Enumerates "X" from 1 to num with a comma as the separator.
	\sa MMP_ENUM
*/
#define MMP_ENUM_C(num,X)                        MMP_ENUM_HELPER(num, X, MMP_COMMA)
#define MMP_ENUM_C_HELPER(num,X)                 MMP_ENUM_ ## num (X, MMP_COMMA)

/*! \brief Enumerates from 1 to num on the macro X. */
#define MMP_ENUM_M(num, X, INTER)                  MMP_ENUM_M_HELPER(num, MMP_UNPAR(X), MMP_UNPAR(INTER) )
#define MMP_ENUM_MC(num,X)                         MMP_ENUM_M_HELPER(num, MMP_UNPAR(X), MMP_COMMA)
#define MMP_ENUM_M_HELPER(num, X, INTER)           MMP_ENUM_M_ ## num (MMP_UNPAR(X), MMP_UNPAR(INTER) )

/*! \brief Unparameterizes passed parameters

	If you need to pass an argument containing commas but don't want it
	separated into parameters, use this.
*/
#define MMP_UNPAR(...)							__VA_ARGS__

/*! \brief Repeates "X" (which had better be a macro taking 1 parameter) num times starting from 1.

	\code
	#define DO(N) T##N;
	MMP_REPEAT(3,DO) //--> DO(1) DO(2) DO(3) --> T1; T2; T3;
	\endcode
*/
#define MMP_REPEAT(num, X, ...)                MMP_REPEAT_HELPER(num, MMP_UNPAR(X), ## __VA_ARGS__)
#define MMP_REPEAT_HELPER(num, X, ...)         MMP_REPEAT_ ## num (MMP_UNPAR(X), ## __VA_ARGS__)

/*! \brief Repeates "X" (which had better be a macro taking 1 parameter) num times starting from 0.
	\sa MMP_REPEAT
*/
#ifdef _MSC_VER
# define MMP_REPEAT_Z(num, X, ...)            X MMP_LEFT_PAREN 0, ## __VA_ARGS__ MMP_RIGHT_PAREN MMP_REPEAT_HELPER(num, MMP_UNPAR(X), ## __VA_ARGS__)
#else
# define MMP_REPEAT_Z(num, X, ...)            X (0, ## __VA_ARGS__) MMP_REPEAT_HELPER(num, MMP_UNPAR(X), ## __VA_ARGS__)
#endif

/*! \brief Adds the token(s) X if num is not 0 */
#define MMP_IF(num,X)                            MMP_IF_HELPER(num, MMP_UNPAR(X))
#define MMP_IF_HELPER(num,X)                     MMP_IF_ ## num (MMP_UNPAR(X))

/*! \brief Adds the token(s) X if num is not 0, Y if 0 */
#define MMP_IF_ELSE(num,X,Y)                            MMP_IF_ELSE_HELPER(num, MMP_UNPAR(X), MMP_UNPAR(Y))
#define MMP_IF_ELSE_HELPER(num,X,Y)                     MMP_IF_ELSE_ ## num (MMP_UNPAR(X), MMP_UNPAR(Y))

/*! \brief Adds the token(s) X if num is 0 */
#define MMP_IF_NOT(num,X)                            MMP_IF_NOT_HELPER(num, MMP_UNPAR(X))
#define MMP_IF_NOT_HELPER(num,X)                     MMP_IF_NOT_ ## num (MMP_UNPAR(X))

/*! \brief Adds the token(s) X if num is 1 */
#define MMP_IF_ONE(num,X)                            MMP_IF_ONE_HELPER(num, MMP_UNPAR(X))
#define MMP_IF_ONE_HELPER(num,X)                     MMP_IF_ONE_ ## num (MMP_UNPAR(X))

/*! \brief Inserts a comma token if num is not 0 */
#define MMP_COMMA_IF(num)                        MMP_IF(num, MMP_COMMA)

#define MMP_BOOL(X)                            MMP_BOOL_HELPER(X)
#define MMP_BOOL_HELPER(num)                   MMP_TABLE_BOOL_ ## num

#define MMP_AND(X, Y)                          MMP_AND_HELPER(X, Y)
#define MMP_AND_HELPER(X, Y)                   MMP_TABLE_AND_ ## X ## Y

#define MMP_OR(X, Y)                           MMP_OR_HELPER(X, Y)
#define MMP_OR_HELPER(X, Y)                    MMP_TABLE_OR_ ## X ## Y

#define MMP_NOT(X)                             MMP_NOT_HELPER(X)
#define MMP_NOT_HELPER(X)                      MMP_TABLE_NOT_ ## X

#if !defined(WIN32) && !defined(WIN64)
# define __cdecl
# define __stdcall
# define __fastcall
# define __thiscall
# endif

#define MMP_ALL_CDECL_FUNCTIONAL_VARIANTS(MAX_ARGS, M, ...)       \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 0, 0, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 0, 0, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    0, 1, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    1, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    1, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    1, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __cdecl,    1, 1, 1, 1, ## __VA_ARGS__)

#define MMP_ALL_STDCALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M, ...)     \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 0, 0, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 0, 0, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  0, 1, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  1, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  1, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __stdcall,  1, 1, 1, 0, ## __VA_ARGS__)

#define MMP_ALL_FASTCALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M, ...)    \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 0, 0, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 0, 0, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 0, 1, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 1, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 1, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 1, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __fastcall, 1, 1, 1, 1, ## __VA_ARGS__)

#define MMP_ALL_THISCALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M, ...)    \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 0, 0, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 0, 0, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 0, 1, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 1, 0, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 1, 0, 1, 1, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 1, 1, 1, 0, ## __VA_ARGS__) \
MMP_REPEAT_Z(MAX_ARGS, M, __thiscall, 1, 1, 1, 1, ## __VA_ARGS__)

#if (defined(X64)  || defined(WIN64)) || (!defined(WIN32) && !(defined(WIN64)))
# define MMP_ALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M)  \
		 MMP_ALL_CDECL_FUNCTIONAL_VARIANTS(   MAX_ARGS, M)
#else
# define MMP_ALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M)  \
		 MMP_ALL_CDECL_FUNCTIONAL_VARIANTS(   MAX_ARGS, M) \
         MMP_ALL_STDCALL_FUNCTIONAL_VARIANTS( MAX_ARGS, M) \
         MMP_ALL_FASTCALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M) \
         MMP_ALL_THISCALL_FUNCTIONAL_VARIANTS(MAX_ARGS, M)
#endif

#define MMP_ENUM_0(X,  INTER)
#define MMP_ENUM_1(X,  INTER) X##1
#define MMP_ENUM_2(X,  INTER) MMP_ENUM_1(X,  MMP_UNPAR(INTER)) INTER X##2
#define MMP_ENUM_3(X,  INTER) MMP_ENUM_2(X,  MMP_UNPAR(INTER)) INTER X##3
#define MMP_ENUM_4(X,  INTER) MMP_ENUM_3(X,  MMP_UNPAR(INTER)) INTER X##4
#define MMP_ENUM_5(X,  INTER) MMP_ENUM_4(X,  MMP_UNPAR(INTER)) INTER X##5
#define MMP_ENUM_6(X,  INTER) MMP_ENUM_5(X,  MMP_UNPAR(INTER)) INTER X##6
#define MMP_ENUM_7(X,  INTER) MMP_ENUM_6(X,  MMP_UNPAR(INTER)) INTER X##7
#define MMP_ENUM_8(X,  INTER) MMP_ENUM_7(X,  MMP_UNPAR(INTER)) INTER X##8
#define MMP_ENUM_9(X,  INTER) MMP_ENUM_8(X,  MMP_UNPAR(INTER)) INTER X##9
#define MMP_ENUM_10(X, INTER) MMP_ENUM_9(X,  MMP_UNPAR(INTER)) INTER X##10
#define MMP_ENUM_11(X, INTER) MMP_ENUM_10(X, MMP_UNPAR(INTER)) INTER X##11
#define MMP_ENUM_12(X, INTER) MMP_ENUM_11(X, MMP_UNPAR(INTER)) INTER X##12
#define MMP_ENUM_13(X, INTER) MMP_ENUM_12(X, MMP_UNPAR(INTER)) INTER X##13
#define MMP_ENUM_14(X, INTER) MMP_ENUM_13(X, MMP_UNPAR(INTER)) INTER X##14
#define MMP_ENUM_15(X, INTER) MMP_ENUM_14(X, MMP_UNPAR(INTER)) INTER X##15
#define MMP_ENUM_16(X, INTER) MMP_ENUM_15(X, MMP_UNPAR(INTER)) INTER X##16
#define MMP_ENUM_17(X, INTER) MMP_ENUM_16(X, MMP_UNPAR(INTER)) INTER X##17
#define MMP_ENUM_18(X, INTER) MMP_ENUM_17(X, MMP_UNPAR(INTER)) INTER X##18
#define MMP_ENUM_19(X, INTER) MMP_ENUM_18(X, MMP_UNPAR(INTER)) INTER X##19
#define MMP_ENUM_20(X, INTER) MMP_ENUM_19(X, MMP_UNPAR(INTER)) INTER X##20
#define MMP_ENUM_21(X, INTER) MMP_ENUM_20(X, MMP_UNPAR(INTER)) INTER X##21
#define MMP_ENUM_22(X, INTER) MMP_ENUM_21(X, MMP_UNPAR(INTER)) INTER X##22
#define MMP_ENUM_23(X, INTER) MMP_ENUM_22(X, MMP_UNPAR(INTER)) INTER X##23
#define MMP_ENUM_24(X, INTER) MMP_ENUM_23(X, MMP_UNPAR(INTER)) INTER X##24
#define MMP_ENUM_25(X, INTER) MMP_ENUM_24(X, MMP_UNPAR(INTER)) INTER X##25
#define MMP_ENUM_26(X, INTER) MMP_ENUM_25(X, MMP_UNPAR(INTER)) INTER X##26
#define MMP_ENUM_27(X, INTER) MMP_ENUM_26(X, MMP_UNPAR(INTER)) INTER X##27
#define MMP_ENUM_28(X, INTER) MMP_ENUM_27(X, MMP_UNPAR(INTER)) INTER X##28
#define MMP_ENUM_29(X, INTER) MMP_ENUM_28(X, MMP_UNPAR(INTER)) INTER X##29
#define MMP_ENUM_30(X, INTER) MMP_ENUM_29(X, MMP_UNPAR(INTER)) INTER X##30
#define MMP_ENUM_31(X, INTER) MMP_ENUM_30(X, MMP_UNPAR(INTER)) INTER X##31
#define MMP_ENUM_32(X, INTER) MMP_ENUM_31(X, MMP_UNPAR(INTER)) INTER X##32
#define MMP_ENUM_MAX(X, INTER) MMP_ENUM_31(X, INTER) /* Change this up to 32 */

#define MMP_ENUM_M_0(X, INTER)
#define MMP_ENUM_M_1(X, INTER)  X(1)
#define MMP_ENUM_M_2(X, INTER)  MMP_ENUM_M_1(X,  MMP_UNPAR(INTER)) INTER X(2)
#define MMP_ENUM_M_3(X, INTER)  MMP_ENUM_M_2(X,  MMP_UNPAR(INTER)) INTER X(3)
#define MMP_ENUM_M_4(X, INTER)  MMP_ENUM_M_3(X,  MMP_UNPAR(INTER)) INTER X(4)
#define MMP_ENUM_M_5(X, INTER)  MMP_ENUM_M_4(X,  MMP_UNPAR(INTER)) INTER X(5)
#define MMP_ENUM_M_6(X, INTER)  MMP_ENUM_M_5(X,  MMP_UNPAR(INTER)) INTER X(6)
#define MMP_ENUM_M_7(X, INTER)  MMP_ENUM_M_6(X,  MMP_UNPAR(INTER)) INTER X(7)
#define MMP_ENUM_M_8(X, INTER)  MMP_ENUM_M_7(X,  MMP_UNPAR(INTER)) INTER X(8)
#define MMP_ENUM_M_9(X, INTER)  MMP_ENUM_M_8(X,  MMP_UNPAR(INTER)) INTER X(9)
#define MMP_ENUM_M_10(X, INTER) MMP_ENUM_M_9(X,  MMP_UNPAR(INTER)) INTER X(10)
#define MMP_ENUM_M_11(X, INTER) MMP_ENUM_M_10(X, MMP_UNPAR(INTER)) INTER X(11)
#define MMP_ENUM_M_12(X, INTER) MMP_ENUM_M_11(X, MMP_UNPAR(INTER)) INTER X(12)
#define MMP_ENUM_M_13(X, INTER) MMP_ENUM_M_12(X, MMP_UNPAR(INTER)) INTER X(13)
#define MMP_ENUM_M_14(X, INTER) MMP_ENUM_M_13(X, MMP_UNPAR(INTER)) INTER X(14)
#define MMP_ENUM_M_15(X, INTER) MMP_ENUM_M_14(X, MMP_UNPAR(INTER)) INTER X(15)
#define MMP_ENUM_M_16(X, INTER) MMP_ENUM_M_15(X, MMP_UNPAR(INTER)) INTER X(16)
#define MMP_ENUM_M_17(X, INTER) MMP_ENUM_M_16(X, MMP_UNPAR(INTER)) INTER X(17)
#define MMP_ENUM_M_18(X, INTER) MMP_ENUM_M_17(X, MMP_UNPAR(INTER)) INTER X(18)
#define MMP_ENUM_M_19(X, INTER) MMP_ENUM_M_18(X, MMP_UNPAR(INTER)) INTER X(19)
#define MMP_ENUM_M_20(X, INTER) MMP_ENUM_M_19(X, MMP_UNPAR(INTER)) INTER X(20)
#define MMP_ENUM_M_21(X, INTER) MMP_ENUM_M_20(X, MMP_UNPAR(INTER)) INTER X(21)
#define MMP_ENUM_M_22(X, INTER) MMP_ENUM_M_21(X, MMP_UNPAR(INTER)) INTER X(22)
#define MMP_ENUM_M_23(X, INTER) MMP_ENUM_M_22(X, MMP_UNPAR(INTER)) INTER X(23)
#define MMP_ENUM_M_24(X, INTER) MMP_ENUM_M_23(X, MMP_UNPAR(INTER)) INTER X(24)
#define MMP_ENUM_M_25(X, INTER) MMP_ENUM_M_24(X, MMP_UNPAR(INTER)) INTER X(25)
#define MMP_ENUM_M_26(X, INTER) MMP_ENUM_M_25(X, MMP_UNPAR(INTER)) INTER X(26)
#define MMP_ENUM_M_27(X, INTER) MMP_ENUM_M_26(X, MMP_UNPAR(INTER)) INTER X(27)
#define MMP_ENUM_M_28(X, INTER) MMP_ENUM_M_27(X, MMP_UNPAR(INTER)) INTER X(28)
#define MMP_ENUM_M_29(X, INTER) MMP_ENUM_M_28(X, MMP_UNPAR(INTER)) INTER X(29)
#define MMP_ENUM_M_30(X, INTER) MMP_ENUM_M_29(X, MMP_UNPAR(INTER)) INTER X(30)
#define MMP_ENUM_M_31(X, INTER) MMP_ENUM_M_30(X, MMP_UNPAR(INTER)) INTER X(31)
#define MMP_ENUM_M_32(X, INTER) MMP_ENUM_M_31(X, MMP_UNPAR(INTER)) INTER X(32)
#define MMP_ENUM_M_MAX(X, INTER) MMP_ENUM_M_31(X, INTER) /* Change this up to 32 */

#ifdef _MSC_VER
# define MMP_REPEAT_0(X, ...)
# define MMP_REPEAT_1( X, ...)                               X MMP_LEFT_PAREN  1, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_2( X, ...) MMP_REPEAT_1( X, __VA_ARGS__) X MMP_LEFT_PAREN  2, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_3( X, ...) MMP_REPEAT_2( X, __VA_ARGS__) X MMP_LEFT_PAREN  3, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_4( X, ...) MMP_REPEAT_3( X, __VA_ARGS__) X MMP_LEFT_PAREN  4, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_5( X, ...) MMP_REPEAT_4( X, __VA_ARGS__) X MMP_LEFT_PAREN  5, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_6( X, ...) MMP_REPEAT_5( X, __VA_ARGS__) X MMP_LEFT_PAREN  6, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_7( X, ...) MMP_REPEAT_6( X, __VA_ARGS__) X MMP_LEFT_PAREN  7, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_8( X, ...) MMP_REPEAT_7( X, __VA_ARGS__) X MMP_LEFT_PAREN  8, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_9( X, ...) MMP_REPEAT_8( X, __VA_ARGS__) X MMP_LEFT_PAREN  9, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_10(X, ...) MMP_REPEAT_9( X, __VA_ARGS__) X MMP_LEFT_PAREN 10, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_11(X, ...) MMP_REPEAT_10(X, __VA_ARGS__) X MMP_LEFT_PAREN 11, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_12(X, ...) MMP_REPEAT_11(X, __VA_ARGS__) X MMP_LEFT_PAREN 12, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_13(X, ...) MMP_REPEAT_12(X, __VA_ARGS__) X MMP_LEFT_PAREN 13, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_14(X, ...) MMP_REPEAT_13(X, __VA_ARGS__) X MMP_LEFT_PAREN 14, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_15(X, ...) MMP_REPEAT_14(X, __VA_ARGS__) X MMP_LEFT_PAREN 15, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_16(X, ...) MMP_REPEAT_15(X, __VA_ARGS__) X MMP_LEFT_PAREN 16, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_17(X, ...) MMP_REPEAT_16(X, __VA_ARGS__) X MMP_LEFT_PAREN 17, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_18(X, ...) MMP_REPEAT_17(X, __VA_ARGS__) X MMP_LEFT_PAREN 18, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_19(X, ...) MMP_REPEAT_18(X, __VA_ARGS__) X MMP_LEFT_PAREN 19, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_20(X, ...) MMP_REPEAT_19(X, __VA_ARGS__) X MMP_LEFT_PAREN 20, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_21(X, ...) MMP_REPEAT_20(X, __VA_ARGS__) X MMP_LEFT_PAREN 21, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_22(X, ...) MMP_REPEAT_21(X, __VA_ARGS__) X MMP_LEFT_PAREN 22, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_23(X, ...) MMP_REPEAT_22(X, __VA_ARGS__) X MMP_LEFT_PAREN 23, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_24(X, ...) MMP_REPEAT_23(X, __VA_ARGS__) X MMP_LEFT_PAREN 24, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_25(X, ...) MMP_REPEAT_24(X, __VA_ARGS__) X MMP_LEFT_PAREN 25, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_26(X, ...) MMP_REPEAT_25(X, __VA_ARGS__) X MMP_LEFT_PAREN 26, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_27(X, ...) MMP_REPEAT_26(X, __VA_ARGS__) X MMP_LEFT_PAREN 27, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_28(X, ...) MMP_REPEAT_27(X, __VA_ARGS__) X MMP_LEFT_PAREN 28, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_29(X, ...) MMP_REPEAT_28(X, __VA_ARGS__) X MMP_LEFT_PAREN 29, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_30(X, ...) MMP_REPEAT_29(X, __VA_ARGS__) X MMP_LEFT_PAREN 30, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_31(X, ...) MMP_REPEAT_30(X, __VA_ARGS__) X MMP_LEFT_PAREN 31, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_32(X, ...) MMP_REPEAT_31(X, __VA_ARGS__) X MMP_LEFT_PAREN 32, ## __VA_ARGS__ MMP_RIGHT_PAREN
# define MMP_REPEAT_MAX(X, ...) MMP_REPEAT_32(X, ## __VA_ARGS__)  /* Change this up to 32 */
#else
# define MMP_REPEAT_0(X, ...)
# define MMP_REPEAT_1( X, ...)                               X (  1, ## __VA_ARGS__ )
# define MMP_REPEAT_2( X, ...) MMP_REPEAT_1( X, __VA_ARGS__) X (  2, ## __VA_ARGS__ )
# define MMP_REPEAT_3( X, ...) MMP_REPEAT_2( X, __VA_ARGS__) X (  3, ## __VA_ARGS__ )
# define MMP_REPEAT_4( X, ...) MMP_REPEAT_3( X, __VA_ARGS__) X (  4, ## __VA_ARGS__ )
# define MMP_REPEAT_5( X, ...) MMP_REPEAT_4( X, __VA_ARGS__) X (  5, ## __VA_ARGS__ )
# define MMP_REPEAT_6( X, ...) MMP_REPEAT_5( X, __VA_ARGS__) X (  6, ## __VA_ARGS__ )
# define MMP_REPEAT_7( X, ...) MMP_REPEAT_6( X, __VA_ARGS__) X (  7, ## __VA_ARGS__ )
# define MMP_REPEAT_8( X, ...) MMP_REPEAT_7( X, __VA_ARGS__) X (  8, ## __VA_ARGS__ )
# define MMP_REPEAT_9( X, ...) MMP_REPEAT_8( X, __VA_ARGS__) X (  9, ## __VA_ARGS__ )
# define MMP_REPEAT_10(X, ...) MMP_REPEAT_9( X, __VA_ARGS__) X ( 10, ## __VA_ARGS__ )
# define MMP_REPEAT_11(X, ...) MMP_REPEAT_10(X, __VA_ARGS__) X ( 11, ## __VA_ARGS__ )
# define MMP_REPEAT_12(X, ...) MMP_REPEAT_11(X, __VA_ARGS__) X ( 12, ## __VA_ARGS__ )
# define MMP_REPEAT_13(X, ...) MMP_REPEAT_12(X, __VA_ARGS__) X ( 13, ## __VA_ARGS__ )
# define MMP_REPEAT_14(X, ...) MMP_REPEAT_13(X, __VA_ARGS__) X ( 14, ## __VA_ARGS__ )
# define MMP_REPEAT_15(X, ...) MMP_REPEAT_14(X, __VA_ARGS__) X ( 15, ## __VA_ARGS__ )
# define MMP_REPEAT_16(X, ...) MMP_REPEAT_15(X, __VA_ARGS__) X ( 16, ## __VA_ARGS__ )
# define MMP_REPEAT_17(X, ...) MMP_REPEAT_16(X, __VA_ARGS__) X ( 17, ## __VA_ARGS__ )
# define MMP_REPEAT_18(X, ...) MMP_REPEAT_17(X, __VA_ARGS__) X ( 18, ## __VA_ARGS__ )
# define MMP_REPEAT_19(X, ...) MMP_REPEAT_18(X, __VA_ARGS__) X ( 19, ## __VA_ARGS__ )
# define MMP_REPEAT_20(X, ...) MMP_REPEAT_19(X, __VA_ARGS__) X ( 20, ## __VA_ARGS__ )
# define MMP_REPEAT_21(X, ...) MMP_REPEAT_20(X, __VA_ARGS__) X ( 21, ## __VA_ARGS__ )
# define MMP_REPEAT_22(X, ...) MMP_REPEAT_21(X, __VA_ARGS__) X ( 22, ## __VA_ARGS__ )
# define MMP_REPEAT_23(X, ...) MMP_REPEAT_22(X, __VA_ARGS__) X ( 23, ## __VA_ARGS__ )
# define MMP_REPEAT_24(X, ...) MMP_REPEAT_23(X, __VA_ARGS__) X ( 24, ## __VA_ARGS__ )
# define MMP_REPEAT_25(X, ...) MMP_REPEAT_24(X, __VA_ARGS__) X ( 25, ## __VA_ARGS__ )
# define MMP_REPEAT_26(X, ...) MMP_REPEAT_25(X, __VA_ARGS__) X ( 26, ## __VA_ARGS__ )
# define MMP_REPEAT_27(X, ...) MMP_REPEAT_26(X, __VA_ARGS__) X ( 27, ## __VA_ARGS__ )
# define MMP_REPEAT_28(X, ...) MMP_REPEAT_27(X, __VA_ARGS__) X ( 28, ## __VA_ARGS__ )
# define MMP_REPEAT_29(X, ...) MMP_REPEAT_28(X, __VA_ARGS__) X ( 29, ## __VA_ARGS__ )
# define MMP_REPEAT_30(X, ...) MMP_REPEAT_29(X, __VA_ARGS__) X ( 30, ## __VA_ARGS__ )
# define MMP_REPEAT_31(X, ...) MMP_REPEAT_30(X, __VA_ARGS__) X ( 31, ## __VA_ARGS__ )
# define MMP_REPEAT_32(X, ...) MMP_REPEAT_31(X, __VA_ARGS__) X ( 32, ## __VA_ARGS__ )
# define MMP_REPEAT_MAX(X, ...) MMP_REPEAT_32(X, ## __VA_ARGS__)  /* Change this up to 32 */
#endif

#define MMP_IF_0(X)
#define MMP_IF_1(X)  X
#define MMP_IF_2(X)  X
#define MMP_IF_3(X)  X
#define MMP_IF_4(X)  X
#define MMP_IF_5(X)  X
#define MMP_IF_6(X)  X
#define MMP_IF_7(X)  X
#define MMP_IF_8(X)  X
#define MMP_IF_9(X)  X
#define MMP_IF_10(X) X
#define MMP_IF_11(X) X
#define MMP_IF_12(X) X
#define MMP_IF_13(X) X
#define MMP_IF_14(X) X
#define MMP_IF_15(X) X
#define MMP_IF_16(X) X
#define MMP_IF_17(X) X
#define MMP_IF_18(X) X
#define MMP_IF_19(X) X
#define MMP_IF_20(X) X
#define MMP_IF_21(X) X
#define MMP_IF_22(X) X
#define MMP_IF_23(X) X
#define MMP_IF_24(X) X
#define MMP_IF_25(X) X
#define MMP_IF_26(X) X
#define MMP_IF_27(X) X
#define MMP_IF_28(X) X
#define MMP_IF_29(X) X
#define MMP_IF_30(X) X
#define MMP_IF_31(X) X
#define MMP_IF_32(X) X

#define MMP_IF_ELSE_0( X,Y) Y
#define MMP_IF_ELSE_1( X,Y) X
#define MMP_IF_ELSE_2( X,Y) X
#define MMP_IF_ELSE_3( X,Y) X
#define MMP_IF_ELSE_4( X,Y) X
#define MMP_IF_ELSE_5( X,Y) X
#define MMP_IF_ELSE_6( X,Y) X
#define MMP_IF_ELSE_7( X,Y) X
#define MMP_IF_ELSE_8( X,Y) X
#define MMP_IF_ELSE_9( X,Y) X
#define MMP_IF_ELSE_10(X,Y) X
#define MMP_IF_ELSE_11(X,Y) X
#define MMP_IF_ELSE_12(X,Y) X
#define MMP_IF_ELSE_13(X,Y) X
#define MMP_IF_ELSE_14(X,Y) X
#define MMP_IF_ELSE_15(X,Y) X
#define MMP_IF_ELSE_16(X,Y) X
#define MMP_IF_ELSE_17(X,Y) X
#define MMP_IF_ELSE_18(X,Y) X
#define MMP_IF_ELSE_19(X,Y) X
#define MMP_IF_ELSE_20(X,Y) X
#define MMP_IF_ELSE_21(X,Y) X
#define MMP_IF_ELSE_22(X,Y) X
#define MMP_IF_ELSE_23(X,Y) X
#define MMP_IF_ELSE_24(X,Y) X
#define MMP_IF_ELSE_25(X,Y) X
#define MMP_IF_ELSE_26(X,Y) X
#define MMP_IF_ELSE_27(X,Y) X
#define MMP_IF_ELSE_28(X,Y) X
#define MMP_IF_ELSE_29(X,Y) X
#define MMP_IF_ELSE_30(X,Y) X
#define MMP_IF_ELSE_31(X,Y) X
#define MMP_IF_ELSE_32(X,Y) X

#define MMP_IF_NOT_0(X) X
#define MMP_IF_NOT_1(X)
#define MMP_IF_NOT_2(X)
#define MMP_IF_NOT_3(X)
#define MMP_IF_NOT_4(X)
#define MMP_IF_NOT_5(X)
#define MMP_IF_NOT_6(X)
#define MMP_IF_NOT_7(X)
#define MMP_IF_NOT_8(X)
#define MMP_IF_NOT_9(X)
#define MMP_IF_NOT_10(X)
#define MMP_IF_NOT_11(X)
#define MMP_IF_NOT_12(X)
#define MMP_IF_NOT_13(X)
#define MMP_IF_NOT_14(X)
#define MMP_IF_NOT_15(X)
#define MMP_IF_NOT_16(X)
#define MMP_IF_NOT_17(X)
#define MMP_IF_NOT_18(X)
#define MMP_IF_NOT_19(X)
#define MMP_IF_NOT_20(X)
#define MMP_IF_NOT_21(X)
#define MMP_IF_NOT_22(X)
#define MMP_IF_NOT_23(X)
#define MMP_IF_NOT_24(X)
#define MMP_IF_NOT_25(X)
#define MMP_IF_NOT_26(X)
#define MMP_IF_NOT_27(X)
#define MMP_IF_NOT_28(X)
#define MMP_IF_NOT_29(X)
#define MMP_IF_NOT_30(X)
#define MMP_IF_NOT_31(X)
#define MMP_IF_NOT_32(X)

#define MMP_IF_ONE_0(X)
#define MMP_IF_ONE_1(X) X
#define MMP_IF_ONE_2(X)
#define MMP_IF_ONE_3(X)
#define MMP_IF_ONE_4(X)
#define MMP_IF_ONE_5(X)
#define MMP_IF_ONE_6(X)
#define MMP_IF_ONE_7(X)
#define MMP_IF_ONE_8(X)
#define MMP_IF_ONE_9(X)
#define MMP_IF_ONE_10(X)
#define MMP_IF_ONE_11(X)
#define MMP_IF_ONE_12(X)
#define MMP_IF_ONE_13(X)
#define MMP_IF_ONE_14(X)
#define MMP_IF_ONE_15(X)
#define MMP_IF_ONE_16(X)
#define MMP_IF_ONE_17(X)
#define MMP_IF_ONE_18(X)
#define MMP_IF_ONE_19(X)
#define MMP_IF_ONE_20(X)
#define MMP_IF_ONE_21(X)
#define MMP_IF_ONE_22(X)
#define MMP_IF_ONE_23(X)
#define MMP_IF_ONE_24(X)
#define MMP_IF_ONE_25(X)
#define MMP_IF_ONE_26(X)
#define MMP_IF_ONE_27(X)
#define MMP_IF_ONE_28(X)
#define MMP_IF_ONE_29(X)
#define MMP_IF_ONE_30(X)
#define MMP_IF_ONE_31(X)
#define MMP_IF_ONE_32(X)

#define MMP_TABLE_BOOL_0  0
#define MMP_TABLE_BOOL_1  1
#define MMP_TABLE_BOOL_2  1
#define MMP_TABLE_BOOL_3  1
#define MMP_TABLE_BOOL_4  1
#define MMP_TABLE_BOOL_5  1
#define MMP_TABLE_BOOL_6  1
#define MMP_TABLE_BOOL_7  1
#define MMP_TABLE_BOOL_8  1
#define MMP_TABLE_BOOL_9  1
#define MMP_TABLE_BOOL_10 1
#define MMP_TABLE_BOOL_11 1
#define MMP_TABLE_BOOL_12 1
#define MMP_TABLE_BOOL_13 1
#define MMP_TABLE_BOOL_14 1
#define MMP_TABLE_BOOL_15 1
#define MMP_TABLE_BOOL_16 1
#define MMP_TABLE_BOOL_17 1
#define MMP_TABLE_BOOL_18 1
#define MMP_TABLE_BOOL_19 1
#define MMP_TABLE_BOOL_20 1
#define MMP_TABLE_BOOL_21 1
#define MMP_TABLE_BOOL_22 1
#define MMP_TABLE_BOOL_23 1
#define MMP_TABLE_BOOL_24 1
#define MMP_TABLE_BOOL_25 1
#define MMP_TABLE_BOOL_26 1
#define MMP_TABLE_BOOL_27 1
#define MMP_TABLE_BOOL_28 1
#define MMP_TABLE_BOOL_29 1
#define MMP_TABLE_BOOL_30 1
#define MMP_TABLE_BOOL_31 1
#define MMP_TABLE_BOOL_32 1

#define MMP_TABLE_AND_00 0
#define MMP_TABLE_AND_01 0
#define MMP_TABLE_AND_10 0
#define MMP_TABLE_AND_11 1

#define MMP_TABLE_OR_00 0
#define MMP_TABLE_OR_01 1
#define MMP_TABLE_OR_10 1
#define MMP_TABLE_OR_11 1

#define MMP_TABLE_NOT_0 1
#define MMP_TABLE_NOT_1 0

#endif
