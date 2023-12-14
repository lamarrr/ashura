#pragma once

#include "ashura/types.h"
#include <cmath>

namespace ash
{
namespace math
{

constexpr f32 abs(f32 x)
{
  return x >= 0 ? x : -x;
}

// WARNING: the only non-floating-point integral type you should be using this
// for is i64.
template <typename T>
constexpr T lerp(T const &a, T const &b, T t)
{
  return static_cast<T>(a + (b - a) * t);
}

template <typename T>
constexpr T inverse_lerp(T const &a, T const &b, T const &v)
{
  return static_cast<T>(v - a) / static_cast<T>(b - a);
}

// template<typename T>
// constexpr T grid_snap(T const& a, T const& unit){

// (a / unit);

// }

// 	/** Snaps a value to the nearest grid multiple */
// 	template< class T >
// 	UE_NODISCARD static constexpr FORCEINLINE T GridSnap(T Location, T Grid)
// 	{
// 		return (Grid == T{}) ? Location : (Floor((Location + (Grid/(T)2)) /
// Grid) * Grid);
// 	}

/*
 *	Cubic Catmull-Rom Spline interpolation. Based on
 *http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf Curves are
 *guaranteed to pass through the control points and are easily chained together.
 *Equation supports abitrary parameterization. eg. Uniform=0,1,2,3 ; chordal=
 *|Pn - Pn-1| ; centripetal = |Pn - Pn-1|^0.5 P0 - The control point preceding
 *the interpolation range. P1 - The control point starting the interpolation
 *range. P2 - The control point ending the interpolation range. P3 - The control
 *point following the interpolation range. T0-3 - The interpolation parameters
 *for the corresponding control points. T - The interpolation factor in the
 *range 0 to 1. 0 returns P1. 1 returns P2.
 */
// template< class U >
// UE_NODISCARD static constexpr FORCEINLINE_DEBUGGABLE U
// CubicCRSplineInterp(const U& P0, const U& P1, const U& P2, const U& P3, const
// float T0, const float T1, const float T2, const float T3, const float T)
// {
// 	//Based on http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf
// 	float InvT1MinusT0 = 1.0f / (T1 - T0);
// 	U L01 = ( P0 * ((T1 - T) * InvT1MinusT0) ) + ( P1 * ((T - T0) *
// InvT1MinusT0) ); 	float InvT2MinusT1 = 1.0f / (T2 - T1); 	U L12 = ( P1 *
// ((T2 - T) * InvT2MinusT1) ) + ( P2 * ((T - T1) * InvT2MinusT1) ); 	float
// InvT3MinusT2 = 1.0f / (T3 - T2); 	U L23 = ( P2 * ((T3 - T) * InvT3MinusT2)
// ) + ( P3 * ((T - T2) * InvT3MinusT2) );

// 	float InvT2MinusT0 = 1.0f / (T2 - T0);
// 	U L012 = ( L01 * ((T2 - T) * InvT2MinusT0) ) + ( L12 * ((T - T0) *
// InvT2MinusT0) ); 	float InvT3MinusT1 = 1.0f / (T3 - T1); 	U L123 = ( L12 *
// ((T3
// - T) * InvT3MinusT1) ) + ( L23 * ((T - T1) * InvT3MinusT1) );

// 	return  ( ( L012 * ((T2 - T) * InvT2MinusT1) ) + ( L123 * ((T - T1) *
// InvT2MinusT1) ) );
// }

}        // namespace math
}        // namespace ash
