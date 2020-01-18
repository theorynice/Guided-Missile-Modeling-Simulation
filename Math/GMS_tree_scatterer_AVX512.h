

#ifndef __GMS_TREE_SCATTERER_AVX512_H__
#define __GMS_TREE_SCATTERER_AVX512_H__

namespace file_info {

     const unsigned int gGMS_TREE_SCATTERER_AVX512_MAJOR = 1U;
     const unsigned int gGMS_TREE_SCATTERER_AVX512_MINOR = 0U;
     const unsigned int gGMS_TREE_SCATTERER_AVX512_MICRO = 0U;
     const unsigned int gGMS_TREE_SCATTERER_AVX512_FULLVER =
        1000U*gGMS_TREE_SCATTERER_AVX512_MAJOR +
	100U*gGMS_TREE_SCATTERER_AVX512_MINOR  +
	10U*gGMS_TREE_SCATTERER_AVX512_MICRO;
     const char * const pgGMS_TREE_SCATTERER_AVX512_CREATION_DATE = "18-01-2020 11:39 +00200 (SAT 18 JAN 2020 GMT+2)";
     const char * const pgGMS_TREE_SCATTERER_AVX512_BUILD_DATE    = __DATE__ " " __TIME__;
     const char * const pgGMS_TREE_SCATTERER_AVX512_AUTHOR        = " Programmer: Bernard Gingold, contact: beniekg@gmail.com";
     const char * const pgGMS_TREE_SCATTERER_AVX512_SYNOPSYS      = " Model of single tree scatterer( AVX-512 implementation) suitable for computation of radar backscatter.";
}

#include <cstdint>
#include "GMS_config.h"
#include "GMS_avx512vecf32.h"


namespace  gms {

           namespace  math {

	          namespace {

                     const AVX512Vec16 VINC0 = AVX512Vec16{1.0f,2.0f,3.0f,4.0f,
		                                           5.0f,6.0f,7.0f,8.0f,
							   9.0f,10.0f,11.0f,12.0f,
							   13.0f,14.0f,15.0f,16.0f};

		     const AVX512Vec16 VINC1 = AVX512Vec16{17.0f,18.0f,19.0f,20.0f,
		                                           21.0f,22.0f,23.0f,24.0f,
							   25.0f,26.0f,27.0f,28.0f,
							   29.0f,30.0f,31.0f,32.0f};

		     const AVX512Vec16 VINC2 = AVX512Vec16{33.0f,34.0f,35.0f,36.0f,
		                                           37.0f,38.0f,39.0f,40.0f,
							   41.0f,42.0f,43.0f,44.0f,
							   45.0f,46.0f,47.0f,48.0f};

		     const AVX512Vec16 VINC3 = AVX512Vec16{49.0f,50.0f,51.0f,52.0f,
		                                           53.0f,54.0f,55.0f,56.0f,
							   57.0f,58.0f,59.0f,60.0f,
							   61.0f,62.0f,63.0f,64.0f};

		     const AVX512Vec16 ZERO  = AVX512Vec16{0.0f};

		     const AVX512Vec16 TWO   = AVX512Vec16{2.0f};

		     const AVX512Vec16 PI    = AVX512Vec16{3.141592653589793f};
	     }


	      // Low temporal access and spatial locality (cold)  data structure (POD) type

	              struct TSColdAVX512_t {

                         // Number of leaves (represented as an ellipsoidal surface)
			 int32_t nleaves;
			 // Number of branches (represented as an cylindrical volumes)
			 int32_t nbranches;
			 //  Number of simulation steps it is equal to Radar PRF (pulse repetetive frequency)
			 int32_t nsteps;
			 //  Tree scatterer ordinal number (for the forest simulation)
			 int32_t ordinal;
			 // Number of parametric equation evaluation 'points' for the trunk cylindrical approximation
                         int32_t trunk_param_npts;
			 //  Number of parametric equation evaluation 'points' for the leaves elliptical approximation
			 int32_t leaves_param_npts;
			 //   Number of parametric equation evaluation 'points' for the branches cylindrical approximation
			 int32_t branches_param_npts;
			 //  Total height of tree
			 float   tree_height;
			 //   Height of the trunk only
			 float   trunk_height;
			 //   Radius of trunk  (averaged)
			 float   trunk_radius;
			 //    Height of crown only
			 float   crown_height;
			 //    Total crown area (approximated) as sum of leaves area
			 float   crown_area;
			 //    Trunk area (cylinder area)
			 float   trunk_area;
			 //    Total tree area
			 float   tree_area;
			 //     Tree geo-location latitude
			 float   tree_lat;
			 //     Tree geo-location longtitude
			 float   tree_lon; // 1st cache line ends here!
			 //     Tree elevation (above the sea level) (meters)
			 float   tree_elevation; // 2nd cache lines starts here
#if (USE_STRUCT_PADDING) == 1
                         PAD_TO_ALIGNED(4,0,4)
#endif
                         // Is water or rather moistness present or not on the branch surface (per n-branches) (allowed values only 0,1)
			 int32_t * __restrict __ATTR_ALIGN__(64) leaves_moist;
			 // Is water or rather moistness present or not on the branch surface (per n-branches) (allowed values only 0,1)
			 int32_t * __restrict __ATTR_ALIGN__(64) branches_moist;
			 //  ! Trunk parametric equation (approximated as a cylindrical object)
                         //   PAOS type size of array -- npoints/16
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) trunk_xparam;
			 //   ! PAOS type size of array -- npoints/8
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) trunk_yparam;
			 //    ! PAOS type size of array -- npoints/8
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) trunk_zparam;
			 //    ! Leaves thicknes (micron) per leaf
                         //    ! PAOS type size of array nleaves/8
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_thick;
			 //    ! Leaves density (g/cm^3) per leaf
                         //    ! PAOS type size of array nleaves/8
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_dens;
			 //    ! Leaves surface angle to inpinging Radar waveform (rad)
                         //    ! PAOS type size of array nleaves/8
			 //     (theta,phi angles)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_incang;
			 //  ! Leaves parameteric equation (approximated as an ellipses)
                         //  ! Parameter x,(a*cos(t))
        
                         //  ! PAOS type size of arrays  1st dim (evaluation of x) ,
                         //  !  2nd dim (number of leaves)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_xparam;
			 //   ! Parameter y, (b*sin(t))
                         //   ! PAOS type size of arrays is -- npoints/8 1st dim (evaluation of y) ,
                         //   ! nleaves/8 2nd dim (number of leaves)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_yparam;
			 //   Branchess thickness
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_thick;
			 //   Branches density
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_dens;
			 //   Branches incident angle
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_incang;
			 	 //  ! Branches parametric equation (approximated as a cylindrical objects)
                         //  ! Parameter x, (r*cos(t))
                         //  ! PAOS type size of arrays is -- npoints/8 1st dim (evaluation of x) ,
                         //  ! nbranches/8 2nd dim (number of leaves)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_xparam;
			 //  ! Parameter y, (r*sin(t))
                         //  !2nd dimension is a branches number, 1st dimension evaluation of parameter y
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_yparam;
			 //  ! Parameter z, (height)
                         //  !2nd dimension is a branch  number, 1st dimension evaluation of parameter z
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_zparam;

		 } __ATTR_ALIGN__(64);

		  //  ! This is a high termporal and spatial locality data type
                  //  ! These data type members characteristics are varying between each sample of Radar PRF.

		      struct TSHotAVX512_t {

                            // ! Whole tree vibration in x-axis (radians)
                         float tree_xangle;
			 //  Whole tree vibration in y-axis (radians)
			 float tree_yangle;
			 //  Sine of x-axis angle
                         float sin_xangle;
			 //  Cos of x-axis angle
			 float cos_xangle;
			 //  Sin of y-axis angle
			 float sin_yangle;
			 //  Cos of y-angle
			 float cos_yangle;
			 //  ! Tree total cross section (dimensionless)
			 float tree_rcs;
			 //  Crown cross section approximated as sum of leaves cross section.
			 float crown_rcs;
			 // Trunk cross section
			 float trunk_rcs;
#if (USE_STRUCT_PADDING)
                         PAD_TO_ALIGNED(4,0,28)
#endif
                         //  ! Leaves cross section (varying due to leaves vibrations)
                         //  ! ( 1st dimension cross section variation per leaf,2nd dimension is PRF/s )
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_rcs;
			 //  ! Leaves reflectivity (varying due to leaves vibration)
                         //  ! ( 1st dimension reflectivity(dbm) pers leaf, 2nd dimension is PRF/s,)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_reflect;
			 //  ! Branches cross section (varying due to branches vibrations)
                         // ! ( 1st dimension cross section variation per branch, 2nd dimension is PRF/s))
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_rcs;
			 //   ! Branches reflectivity (varying due to leaves vibration)
                         //     ! ( 1st dimension reflectivity(dbm) pers branch, 2nd dimension is PRF/s))
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_reflect;
			 //   ! Leaves angle of vibration in x-axis per PRF/s
                         //   ! 1st dimension angle values (rad),  2nd dimension PRF/s,
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_xang;
		         //   ! Leaves sine of vibration angle in x-axis per PRF/s
                         //   ! 1st  dimension sine of vibrational angle (rad),  2nd dimension PRF/s,
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_sin_xang;
			 //     ! Leaves sine of vibration angle in x-axis per PRF/s
                         //     ! 1st dimension PRF/s, 2nd dimension sine of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_cos_xang;
                         	 //      ! Leaves angle of vibration in y-axis per PRF/s
                         //      ! 1st dimension PRF/s, 2nd dimension angle values (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_yang;
			 //      ! Leaves sine of vibration angle in y-axis per PRF/s
                         //      ! 1st dimension PRF/s, 2nd dimension angle of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_sin_yang;
			 //        ! Leaves sine of vibration angle in y-axis per PRF/s
                         //        ! 1st dimension PRF/s, 2nd dimension sine of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) leaves_cos_yang;
			 // ! Branches angle of vibration in x-axis per PRF/s
                         // ! 1st dimension PRF/s, 2nd dimension angle values (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_xang;
			 //  ! Branches sine of vibration angle in x-axis per PRF/s
                         //  ! 1st dimension PRF/s, 2nd dimension angle  of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_sin_xang;
			 //   ! Branches cosine of vibration angle in x-axis per PRF/s
                         //   ! 1st dimension PRF/s, 2nd dimension cosine of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_cos_xang;
			 //   
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_yang;
			 //  ! Branches sine of vibration angle in y-axis per PRF/s
                         //  ! 1st dimension PRF/s, 2nd dimension sine  of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_sin_yang;
			 //  ! Branches cosine of vibration angle in x-axis per PRF/s
                         //   ! 1st dimension PRF/s, 2nd dimension cosine  of vibrational angle (rad)
			 AVX512Vec16 * __restrict __ATTR_ALIGN__(64) branches_cos_yang;
                         
		 } __ATTR_ALIGN__(64);


		      struct TreeScattererAVX512 {

                          TSColdAVX512_t m_tsc __ATTR_ALIGN__(64);
			  
			  TSHotAVX512_t  m_tsh __ATTR_ALIGN__(64);

			  TreeScattererAVX512() __ATTR_COLD__ __ATTR_ALIGN__(32);

			  TreeScattererAVX512(const int32_t,
		                              const int32_t,
				              const int32_t,
				              const int32_t,
				              const int32_t,
				              const int32_t,
				              const int32_t,
				              const float,
				              const float,
				              const float,
				              const float,
				              const float,
				              const float,
				              const float)  __ATTR_COLD__ __ATTR_ALIGN__(32);

			  TreeScattererAVX512(const TreeScattererAVX512 &) = delete;

			  TreeScattererAVX512( TreeScattererAVX512 &&) = delete;

			  ~TreeScattererAVX512() noexcept(true);

			  TreeScattererAVX512 &
			  operator=(const TreeScattererAVX512 &) = delete;

			  TreeScattererAVX512 &
			  operator=( TreeScattererAVX512 &&) = delete;

			  void SetMoistness_scalar() __ATTR_COLD__ __ATTR_ALIGN__(32);

			  void ComputeTrunkParamEq_zmm16r4(const int32_t)  __ATTR_COLD__ __ATTR_ALIGN__(32);

			  void SetThickDensAng_zmm16r4(const AVX512Vec16 * __restrict __ATTR_ALIGN__(64)) __ATTR_COLD__ __ATTR_ALIGN__(32);

			  void ComputeLeavesParamEq_zmm16r4(
#if defined __ICC || defined __INTEL_COMPILER
                                                            const AVX512Vec16,
							    const AVX512Vec16
#elif defined __GNUC__ && !defined __INTEL_COMPILER
                                                            const float,
							    const float
#endif
                                                            ) __ATTR_COLD__ __ATTR_ALIGN__(32);
							    
			   void ComputeBranchesParamEq_zmm16r4(
#if defined __ICC || defined __INTEL_COMPILER
                                                               const AVX512Vec16,
							       const AVX512Vec16,
							       const int32_t
#elif defined __GNUC__ && !defined __INTEL_COMPILER
                                                               const float,
							       const float,
							       const int32_t
#endif
                                                               ) __ATTR_COLD__ __ATTR_ALIGN__(32);
			  

		} __ATTR_ALIGN__(64);

      } // math


} // gms













#endif /*__GMS_TREE_SCATTERER_AVX512_H__*/
