#ifndef _JZMXU32__H_
#define _JZMXU32__H_


/* =================== Multiplication =================== */

#undef S32MUL
#define S32MUL(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32mul %0,%1,%2,%3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef S32MULU
#define S32MULU(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32mulu %0,%1,%2,%3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef S32MADD
#define S32MADD(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32madd %0,%1,%2,%3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef S32MADDU
#define S32MADDU(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32maddu %0,%1,%2,%3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef S32MSUB
#define S32MSUB(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32msub %0,%1,%2,%3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef S32MSUBU
#define S32MSUBU(xra,xrd,rs,rt)				\
  do {							\
    __asm__ __volatile ("s32msubu %0,%1,%2,%3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d" ((rs)),"d"((rt))		\
			:"hi","lo");			\
  } while (0)

#undef D16MUL_WW
#define D16MUL_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mul %0,%2,%3,%1,0"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MUL_LW
#define D16MUL_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mul %0,%2,%3,%1,1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MUL_HW
#define D16MUL_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mul %0,%2,%3,%1,2"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MUL_XW
#define D16MUL_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mul %0,%2,%3,%1,3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULF_WW
#define D16MULF_WW(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16mulf %0,%1,%2,0"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULF_LW
#define D16MULF_LW(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16mulf %0,%1,%2,1"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULF_HW
#define D16MULF_HW(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16mulf %0,%1,%2,2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULF_XW
#define D16MULF_XW(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16mulf %0,%1,%2,3"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULE_WW
#define D16MULE_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mule %0,%2,%3,%1,0"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULE_LW
#define D16MULE_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mule %0,%2,%3,%1,1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULE_HW
#define D16MULE_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mule %0,%2,%3,%1,2"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MULE_XW
#define D16MULE_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mule %0,%2,%3,%1,3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AA_WW
#define D16MAC_AA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,0,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AA_LW
#define D16MAC_AA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,0,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AA_HW
#define D16MAC_AA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,0,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AA_XW
#define D16MAC_AA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,0,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AS_WW
#define D16MAC_AS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,1,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AS_LW
#define D16MAC_AS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,1,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AS_HW
#define D16MAC_AS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,1,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_AS_XW
#define D16MAC_AS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,1,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SA_WW
#define D16MAC_SA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,2,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SA_LW
#define D16MAC_SA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,2,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SA_HW
#define D16MAC_SA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,2,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SA_XW
#define D16MAC_SA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,2,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SS_WW
#define D16MAC_SS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,3,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SS_LW
#define D16MAC_SS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,3,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SS_HW
#define D16MAC_SS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,3,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAC_SS_XW
#define D16MAC_SS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mac %0,%2,%3,%1,3,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACF_AA_WW
#define D16MACF_AA_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,0,0"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AA_LW
#define D16MACF_AA_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,0,1"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AA_HW
#define D16MACF_AA_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,0,2"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AA_XW
#define D16MACF_AA_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,0,3"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AS_WW
#define D16MACF_AS_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,1,0"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AS_LW
#define D16MACF_AS_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,1,1"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AS_HW
#define D16MACF_AS_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,1,2"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_AS_XW
#define D16MACF_AS_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,1,3"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SA_WW
#define D16MACF_SA_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,2,0"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SA_LW
#define D16MACF_SA_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,2,1"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SA_HW
#define D16MACF_SA_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,2,2"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SA_XW
#define D16MACF_SA_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,2,3"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SS_WW
#define D16MACF_SS_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,3,0"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SS_LW
#define D16MACF_SS_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,3,1"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SS_HW
#define D16MACF_SS_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,3,2"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACF_SS_XW
#define D16MACF_SS_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16macf %0,%1,%2,%3,3,3"		\
			:"+u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"u"((xrd)));	\
  } while (0)

#undef D16MACE_AA_WW
#define D16MACE_AA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,0,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AA_LW
#define D16MACE_AA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,0,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AA_HW
#define D16MACE_AA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,0,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AA_XW
#define D16MACE_AA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,0,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AS_WW
#define D16MACE_AS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,1,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AS_LW
#define D16MACE_AS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,1,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AS_HW
#define D16MACE_AS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,1,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_AS_XW
#define D16MACE_AS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,1,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SA_WW
#define D16MACE_SA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,2,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SA_LW
#define D16MACE_SA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,2,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SA_HW
#define D16MACE_SA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,2,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SA_XW
#define D16MACE_SA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,2,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SS_WW
#define D16MACE_SS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,3,0"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SS_LW
#define D16MACE_SS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,3,1"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SS_HW
#define D16MACE_SS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,3,2"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MACE_SS_XW
#define D16MACE_SS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16mace %0,%2,%3,%1,3,3"	\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AA_WW
#define D16MADL_AA_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,0,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AA_LW
#define D16MADL_AA_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,0,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AA_HW
#define D16MADL_AA_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,0,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AA_XW
#define D16MADL_AA_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,0,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AS_WW
#define D16MADL_AS_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,1,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AS_LW
#define D16MADL_AS_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,1,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AS_HW
#define D16MADL_AS_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,1,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_AS_XW
#define D16MADL_AS_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,1,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SA_WW
#define D16MADL_SA_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,2,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SA_LW
#define D16MADL_SA_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,2,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SA_HW
#define D16MADL_SA_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,2,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SA_XW
#define D16MADL_SA_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,2,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SS_WW
#define D16MADL_SS_WW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,3,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SS_LW
#define D16MADL_SS_LW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,3,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SS_HW
#define D16MADL_SS_HW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,3,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MADL_SS_XW
#define D16MADL_SS_XW(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("d16madl %1,%2,%3,%0,3,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_A_HH
#define S16MAD_A_HH(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,0,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_A_LL
#define S16MAD_A_LL(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,0,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_A_HL
#define S16MAD_A_HL(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,0,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_A_LH
#define S16MAD_A_LH(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,0,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_S_HH
#define S16MAD_S_HH(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,1,0"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_S_LL
#define S16MAD_S_LL(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,1,1"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_S_HL
#define S16MAD_S_HL(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,1,2"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S16MAD_S_LH
#define S16MAD_S_LH(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("s16mad %1,%2,%3,%0,1,3"		\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MUL
#define Q8MUL(xra,xrb,xrc,xrd)				\
  do {							\
    __asm__ __volatile ("q8mul %0,%2,%3,%1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MULSU
#define Q8MULSU(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8mulsu %0,%2,%3,%1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MAC_AA
#define Q8MAC_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8mac %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MAC_AS
#define Q8MAC_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8mac %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MAC_SA
#define Q8MAC_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8mac %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MAC_SS
#define Q8MAC_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8mac %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MACSU_AA
#define Q8MACSU_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8macsu %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MACSU_AS
#define Q8MACSU_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8macsu %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MACSU_SA
#define Q8MACSU_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8macsu %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MACSU_SS
#define Q8MACSU_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8macsu %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

/* q8madl is obsolete  */
#undef Q8MADL_AA
#define Q8MADL_AA(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("q8madl %1,%2,%3,%0,0"			\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MADL_AS
#define Q8MADL_AS(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("q8madl %1,%2,%3,%0,1"			\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MADL_SA
#define Q8MADL_SA(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("q8madl %1,%2,%3,%0,2"			\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MADL_SS
#define Q8MADL_SS(xra,xrb,xrc,xrd)				\
  do {								\
    __asm__ __volatile ("q8madl %1,%2,%3,%0,3"			\
			:"=u"((xrd))				\
			:"u"((xra)),"u"((xrb)),"u"((xrc)));	\
  } while (0)

/* ============ ADD & SUBSTRACT ============ */

#undef D32ADD_AA
#define D32ADD_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32add %0,%2,%3,%1,0"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ADD_AS
#define D32ADD_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32add %0,%2,%3,%1,1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ADD_SA
#define D32ADD_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32add %0,%2,%3,%1,2"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ADD_SS
#define D32ADD_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32add %0,%2,%3,%1,3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ADDC
#define D32ADDC(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32addc %0,%2,%3,%1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACC_AA
#define D32ACC_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32acc %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACC_AS
#define D32ACC_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32acc %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACC_SA
#define D32ACC_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32acc %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACC_SS
#define D32ACC_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32acc %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACCM_AA
#define D32ACCM_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32accm %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACCM_AS
#define D32ACCM_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32accm %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACCM_SA
#define D32ACCM_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32accm %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ACCM_SS
#define D32ACCM_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32accm %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ASUM_AA
#define D32ASUM_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32asum %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ASUM_AS
#define D32ASUM_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32asum %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ASUM_SA
#define D32ASUM_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32asum %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32ASUM_SS
#define D32ASUM_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d32asum %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32CPS
#define S32CPS(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32cps %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32SLT
#define S32SLT(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32slt %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32MOVZ
#define S32MOVZ(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32movz %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32MOVN
#define S32MOVN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32movn %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AA_WW
#define Q16ADD_AA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,0,0"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AA_LW
#define Q16ADD_AA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,0,1"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AA_HW
#define Q16ADD_AA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,0,2"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AA_XW
#define Q16ADD_AA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,0,3"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AS_WW
#define Q16ADD_AS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,1,0"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AS_LW
#define Q16ADD_AS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,1,1"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AS_HW
#define Q16ADD_AS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,1,2"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_AS_XW
#define Q16ADD_AS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,1,3"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SA_WW
#define Q16ADD_SA_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,2,0"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SA_LW
#define Q16ADD_SA_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,2,1"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SA_HW
#define Q16ADD_SA_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,2,2"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SA_XW
#define Q16ADD_SA_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,2,3"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SS_WW
#define Q16ADD_SS_WW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,3,0"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SS_LW
#define Q16ADD_SS_LW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,3,1"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SS_HW
#define Q16ADD_SS_HW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,3,2"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ADD_SS_XW
#define Q16ADD_SS_XW(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16add %0,%2,%3,%1,3,3"	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACC_AA
#define Q16ACC_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16acc %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACC_AS
#define Q16ACC_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16acc %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACC_SA
#define Q16ACC_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16acc %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACC_SS
#define Q16ACC_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16acc %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACCM_AA
#define Q16ACCM_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16accm %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACCM_AS
#define Q16ACCM_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16accm %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACCM_SA
#define Q16ACCM_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16accm %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16ACCM_SS
#define Q16ACCM_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16accm %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16ASUM_AA
#define D16ASUM_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16asum %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16ASUM_AS
#define D16ASUM_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16asum %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16ASUM_SA
#define D16ASUM_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16asum %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16ASUM_SS
#define D16ASUM_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("d16asum %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16CPS
#define D16CPS(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16cps %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16SLT
#define D16SLT(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16slt %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MOVZ
#define D16MOVZ(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16movz %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MOVN
#define D16MOVN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16movn %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16AVG
#define D16AVG(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16avg %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16AVGR
#define D16AVGR(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16avgr %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADDE_AA
#define Q8ADDE_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8adde %0,%2,%3,%1,0"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADDE_AS
#define Q8ADDE_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8adde %0,%2,%3,%1,1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADDE_SA
#define Q8ADDE_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8adde %0,%2,%3,%1,2"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADDE_SS
#define Q8ADDE_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8adde %0,%2,%3,%1,3"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

/* q8add is obsolete */
#undef Q8ADD_AA
#define Q8ADD_AA(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8add %0,%1,%2,0"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADD_AS
#define Q8ADD_AS(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8add %0,%1,%2,1"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADD_SA
#define Q8ADD_SA(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8add %0,%1,%2,2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ADD_SS
#define Q8ADD_SS(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8add %0,%1,%2,3"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ACCE_AA
#define Q8ACCE_AA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8acce %0,%2,%3,%1,0"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ACCE_AS
#define Q8ACCE_AS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8acce %0,%2,%3,%1,1"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ACCE_SA
#define Q8ACCE_SA(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8acce %0,%2,%3,%1,2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ACCE_SS
#define Q8ACCE_SS(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q8acce %0,%2,%3,%1,3"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D8SUM
#define D8SUM(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d8sum %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D8SUMC
#define D8SUMC(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d8sumc %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8ABD
#define Q8ABD(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8abd %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8SLT
#define Q8SLT(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8slt %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8SLTU
#define Q8SLTU(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8sltu %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MOVZ
#define Q8MOVZ(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8movz %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MOVN
#define Q8MOVN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8movn %0,%1,%2"		\
			:"+u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8SAD
#define Q8SAD(xra,xrb,xrc,xrd)				\
  do {							\
    __asm__ __volatile ("q8sad %0,%2,%3,%1"		\
			:"=u"((xra)),"+u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8AVG
#define Q8AVG(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8avg %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8AVGR
#define Q8AVGR(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8avgr %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D32SLL
#define D32SLL(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("d32sll %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef D32SLR
#define D32SLR(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("d32slr %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef D32SAR
#define D32SAR(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("d32sar %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef D32SARL
#define D32SARL(xra,xrb,xrc,SFT4)				\
  do {								\
    __asm__ __volatile ("d32sarl %0,%1,%2,%d3"			\
			:"=u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef D32SLLV
#define D32SLLV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("d32sllv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef D32SLRV
#define D32SLRV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("d32slrv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef D32SARV
#define D32SARV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("d32sarv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef D32SARW
#define D32SARW(xra,xrb,xrc,rb)					\
  do {								\
    __asm__ __volatile ("d32sarw %0,%1,%2,%3"			\
			:"=u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"d"((rb)));	\
  } while (0)

#undef Q16SLL
#define Q16SLL(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("q16sll %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef Q16SLR
#define Q16SLR(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("q16slr %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef Q16SAR
#define Q16SAR(xra,xrb,xrc,xrd,SFT4)				\
  do {								\
    __asm__ __volatile ("q16sar %0,%2,%3,%1,%d4"		\
			:"=u"((xra)),"=u"((xrd))		\
			:"u"((xrb)),"u"((xrc)),"i"((SFT4)));	\
  } while (0)

#undef Q16SLLV
#define Q16SLLV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("q16sllv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef Q16SLRV
#define Q16SLRV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("q16slrv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef Q16SARV
#define Q16SARV(xra,xrd,rb)				\
  do {							\
    __asm__ __volatile ("q16sarv %0,%1,%z2"		\
			:"+u"((xra)),"+u"((xrd))	\
			:"d"((rb)));			\
  } while (0)

#undef S32EXTR
#define S32EXTR(xra,xrd,rs,bits5)				\
  do {								\
    __asm__ __volatile ("s32extr %0,%1,%2,%d3"			\
			:"+u"((xra))				\
			:"u"((xrd)),"d"((rs)),"i"((bits5)));	\
  } while (0)

#undef S32EXTRV
#define S32EXTRV(xra,xrd,rs,rt)					\
  do {								\
    __asm__ __volatile ("s32extrv %0,%1,%2,%3"			\
			:"+u"((xra))				\
			:"u"((xrd)),"d"((rs)),"d"((rt)));	\
  } while (0)

/* =============== MAX & MIN =============== */

#undef S32MAX
#define S32MAX(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32max %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32MIN
#define S32MIN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32min %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MAX
#define D16MAX(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16max %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef D16MIN
#define D16MIN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("d16min %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MAX
#define Q8MAX(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8max %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q8MIN
#define Q8MIN(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q8min %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

/* ============ Bitwise ============ */

#undef S32AND
#define S32AND(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32and %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32OR
#define S32OR(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32or %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32XOR
#define S32XOR(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32xor %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32NOR
#define S32NOR(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("s32nor %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

/* ============= S32M2I/S32I2M ============= */
#undef S32M2I
#define S32M2I(xra,rb)				\
  do {						\
    __asm__ __volatile ("s32m2i %1,%0"		\
			:"=d"((rb))		\
			:"u"((xra)));		\
  } while (0)

#undef S32I2M
#define S32I2M(xra,rb)				\
  do {						\
    __asm__ __volatile ("s32i2m %0,%1"		\
			:"=u"((xra))		\
			:"d"((rb)));		\
  } while (0)

/* ============ Miscellaneous ============ */

#undef S32SFL
#define S32SFL(xra,xrb,xrc,xrd,OPTN2)			\
  do {							\
    __asm__ __volatile ("s32sfl %0,%2,%3,%1," #OPTN2	\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32ALN
#define S32ALN(xra,xrb,xrc,rs)					\
  do {								\
    __asm__ __volatile ("s32aln %0,%1,%2,%3"			\
			:"=u"((xra))				\
			:"u"((xrb)),"u"((xrc)),"d"((rs)));	\
  } while (0)

#undef S32ALNI
#define S32ALNI(xra,xrb,xrc,OPTN3)			\
  do {							\
    __asm__ __volatile ("s32alni %0,%1,%2," #OPTN3	\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16SAT
#define Q16SAT(xra,xrb,xrc)				\
  do {							\
    __asm__ __volatile ("q16sat %0,%1,%2"		\
			:"=u"((xra))			\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef Q16SCOP
#define Q16SCOP(xra,xrb,xrc,xrd)			\
  do {							\
    __asm__ __volatile ("q16scop %0,%2,%3,%1"		\
			:"=u"((xra)),"=u"((xrd))	\
			:"u"((xrb)),"u"((xrc)));	\
  } while (0)

#undef S32LUI
#define S32LUI(xra,s8,OPTN3)			\
  do {						\
    __asm__ __volatile ("s32lui %0,%d1," #OPTN3	\
			:"=u"((xra))		\
			:"i"((s8)));		\
  } while (0)

/* ==================Load/Store================== */

#undef S32LDDR
#define S32LDDR(xra,rb,s12)			\
  do {						\
    __asm__ __volatile ("s32lddr %0,%1,%2"	\
			:"=u"((xra))		\
			:"d"((rb)),"i"((s12)));	\
  } while (0)

#undef S32STDR
#define S32STDR(xra,rb,s12)						\
  do {									\
    __asm__ __volatile ("s32stdr %0,%1,%2"				\
			:						\
			:"u"((xra)),"d"((rb)),"i"((s12)):"memory");	\
  } while (0)

#undef S32LDD
#define S32LDD(xra,rb,s12)			\
  do {						\
    __asm__ __volatile ("s32ldd %0,%1,%2"	\
			:"=u"((xra))		\
			:"d"((rb)),"i"((s12)));	\
  } while (0)

#undef S32STD
#define S32STD(xra,rb,s12)						\
  do {									\
    __asm__ __volatile ("s32std %0,%1,%2"				\
			:						\
			:"u"((xra)),"d"((rb)),"i"((s12)):"memory");	\
  } while (0)

#undef S32LDDVR
#define S32LDDVR(xra,rb,rc,STRD2)			\
  do {							\
    __asm__ __volatile ("s32lddvr %0,%1,%2," #STRD2	\
			:"=u"((xra))			\
			:"d"((rb)),"d"((rc)));		\
  } while (0)

#undef S32STDVR
#define S32STDVR(xra,rb,rc,STRD2)					\
  do {									\
    __asm__ __volatile ("s32stdvr %0,%1,%2," #STRD2			\
			:						\
			:"u"((xra)),"d"((rb)),"d"((rc)):"memory");	\
  } while (0)

#undef S32LDDV
#define S32LDDV(xra,rb,rc,STRD2)			\
  do {							\
    __asm__ __volatile ("s32lddv %0,%1,%2," #STRD2	\
			:"=u"((xra))			\
			:"d"((rb)),"d"((rc)));		\
  } while (0)

#undef S32STDV
#define S32STDV(xra,rb,rc,STRD2)					\
  do {									\
    __asm__ __volatile ("s32stdv %0,%1,%2," #STRD2			\
			:						\
			:"u"((xra)),"d"((rb)),"d"((rc)):"memory");	\
  } while (0)

#undef S32LDIR
#define S32LDIR(xra,rb,s12)			\
  do {						\
    __asm__ __volatile ("s32ldir %0,%1,%2"	\
			:"=u"((xra)),"+d"((rb))	\
			:"i"((s12)));		\
  } while (0)

#undef S32SDIR
#define S32SDIR(xra,rb,s12)					\
  do {								\
    __asm__ __volatile ("s32sdir %1,%0,%2"			\
			:"+d"((rb))				\
			:"u"((xra)),"i"((s12)):"memory");	\
  } while (0)

#undef S32LDI
#define S32LDI(xra,rb,s12)			\
  do {						\
    __asm__ __volatile ("s32ldi %0,%1,%2"	\
			:"=u"((xra)),"+d"((rb))	\
			:"i"((s12)));		\
  } while (0)

#undef S32SDI
#define S32SDI(xra,rb,s12)					\
  do {								\
    __asm__ __volatile ("s32sdi %1,%0,%2"			\
			:"+d"((rb))				\
			:"u"((xra)),"i"((s12)):"memory");	\
  } while (0)

#undef S32LDIVR
#define S32LDIVR(xra,rb,rc,STRD2)			\
  do {							\
    __asm__ __volatile ("s32ldivr %0,%1,%2,%3"		\
			:"=u"((xra)),"+d"((rb))		\
			:"d"((rc)),"i"((STRD2)));	\
  } while (0)

#undef S32SDIVR
#define S32SDIVR(xra,rb,rc,STRD2)					\
  do {									\
    __asm__ __volatile ("s32sdivr %1,%0,%2,%3"				\
			:"+d"((rb))					\
			:"u"((xra)),"d"((rc)),"i"((STRD2)):"memory");	\
  } while (0)

#undef S32LDIV
#define S32LDIV(xra,rb,rc,STRD2)			\
  do {							\
    __asm__ __volatile ("s32ldiv %0,%1,%2,%3"		\
			:"=u"((xra)),"+d"((rb))		\
			:"d"((rc)),"i"((STRD2)));	\
  } while (0)

#undef S32SDIV
#define S32SDIV(xra,rb,rc,STRD2)					\
  do {									\
    __asm__ __volatile ("s32sdiv %1,%0,%2,%3"				\
			:"+d"((rb))					\
			:"u"((xra)),"d"((rc)),"i"((STRD2)):"memory");	\
  } while (0)

#undef LXW
#define LXW(rd,rs,rt,STRD2)					\
  do {								\
    __asm__ __volatile ("lxw %0,%1,%2,%3"			\
			:"=d"((rd))				\
			:"d"((rs)),"d"((rt)),"i"((STRD2)));	\
  } while (0)

#undef S16LDD
#define S16LDD(xra,rb,s10,OPTN2)			\
  do {							\
    switch(OPTN2){					\
    case 0:						\
    case 1:						\
      __asm__ __volatile ("s16ldd %0,%1,%2," #OPTN2	\
			  :"+u"((xra))			\
			  :"d"((rb)),"i"((s10)));	\
      break;						\
    case 2:						\
    case 3:						\
      __asm__ __volatile ("s16ldd %0,%1,%2," #OPTN2	\
			  :"=u"((xra))			\
			  :"d"((rb)),"i"((s10)));	\
      break;						\
    }							\
  } while (0)

#undef S16LDI
#define S16LDI(xra,rb,s10,OPTN2)			\
  do {							\
    switch(OPTN2){					\
    case 0:						\
    case 1:						\
      __asm__ __volatile ("s16ldi %0,%1,%2," #OPTN2	\
			  :"+u"((xra)),"+d"((rb))	\
			  :"i"((s10)));			\
      break;						\
    case 2:						\
    case 3:						\
      __asm__ __volatile ("s16ldi %0,%1,%2," #OPTN2	\
			  :"=u"((xra)),"+d"((rb))	\
			  :"i"((s10)));			\
      break;						\
    }							\
  } while (0)

#undef S16STD
#define S16STD(xra,rb,s10,OPTN2)					\
  do {									\
    __asm__ __volatile ("s16std %0,%1,%2," #OPTN2			\
			:						\
			:"u"((xra)),"d"((rb)),"i"((s10)):"memory");	\
  } while (0)

#undef S16SDI
#define S16SDI(xra,rb,s10,OPTN2)				\
  do {								\
    __asm__ __volatile ("s16sdi %1,%0,%2," #OPTN2		\
			:"+d"((rb))				\
			:"u"((xra)),"i"((s10)):"memory");	\
  } while (0)

#undef LXH
#define LXH(rd,rs,rt,STRD2)					\
  do {								\
    __asm__ __volatile ("lxh %0,%1,%2,%3"			\
			:"=d"((rd))				\
			:"d"((rs)),"d"((rt)),"i"((STRD2)));	\
  } while (0)

#undef LXHU
#define LXHU(rd,rs,rt,STRD2)					\
  do {								\
    __asm__ __volatile ("lxhu %0,%1,%2,%3"			\
			:"=d"((rd))				\
			:"d"((rs)),"d"((rt)),"i"((STRD2)));	\
  } while (0)

#undef S8LDD
#define S8LDD(xra,rb,s8,OPTN3)				\
  do {							\
    switch(OPTN3){					\
    case 0:						\
    case 1:						\
    case 2:						\
    case 3:						\
      __asm__ __volatile ("s8ldd %0,%1,%2," #OPTN3	\
			  :"+u"((xra))			\
			  :"d"((rb)),"i"((s8)));	\
      break;						\
    case 4:						\
    case 5:						\
    case 6:						\
    case 7:						\
      __asm__ __volatile ("s8ldd %0,%1,%2," #OPTN3	\
			  :"=u"((xra))			\
			  :"d"((rb)),"i"((s8)));	\
      break;						\
    }							\
  } while (0)

#undef S8LDI
#define S8LDI(xra,rb,s8,OPTN3)				\
  do {							\
    switch(OPTN3){					\
    case 0:						\
    case 1:						\
    case 2:						\
    case 3:						\
      __asm__ __volatile ("s8ldi %0,%1,%2," #OPTN3	\
			  :"+u"((xra)),"+d"((rb))	\
			  :"i"((s8)));			\
      break;						\
    case 4:						\
    case 5:						\
    case 6:						\
    case 7:						\
      __asm__ __volatile ("s8ldi %0,%1,%2," #OPTN3	\
			  :"=u"((xra)),"+d"((rb))	\
			  :"i"((s8)));			\
      break;						\
    }							\
  } while (0)

#undef S8STD
#define S8STD(xra,rb,s8,OPTN3)						\
  do {									\
    __asm__ __volatile ("s8std %0,%1,%2," #OPTN3			\
			:						\
			:"u"((xra)),"d"((rb)),"i"((s8)):"memory");	\
  } while (0)

#undef S8SDI
#define S8SDI(xra,rb,s8,OPTN3)					\
  do {								\
    __asm__ __volatile ("s8sdi %1,%0,%2," #OPTN3		\
			:"+d"((rb))				\
			:"u"((xra)),"i"((s8)):"memory");	\
  } while (0)

#undef LXB
#define LXB(rd,rs,rt,STRD2)					\
  do {								\
    __asm__ __volatile ("lxb %0,%1,%2,%3"			\
			:"=d"((rd))				\
			:"d"((rs)),"d"((rt)),"i"((STRD2)));	\
  } while (0)

#undef LXBU
#define LXBU(rd,rs,rt,STRD2)					\
  do {								\
    __asm__ __volatile ("lxbu %0,%1,%2,%3"			\
			:"=d"((rd))				\
			:"d"((rs)),"d"((rt)),"i"((STRD2)));	\
  } while (0)

#endif	/* _JZMXU32__H_ */
