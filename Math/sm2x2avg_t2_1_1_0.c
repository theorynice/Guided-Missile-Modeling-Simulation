#if (LEAF_PHASE_MATRICES_AUTOVECTORIZE) == 1

         for(k=0; k != 2; ++k) {
#if defined __INTEL_COMPILER
           __assume_aligned(scat2x2m,64);
#pragma vector always
#pragma code_align(64)
#elif defined __GNUC__ && !defined __INTEL_COMPILER
          scat2x2m = (std::complex<float>*)__builtin_assume_aligned(scat2x2m,64);
#pragma omp simd
#endif
                for(l=0; l != 2; ++l) {
                    t1 = sm2x2avg_t2[k][l][0]+orient_distr*scat2x2m[k][l];
		    sm2x2avg_t2[k][l][0] = t1;
		}
	 }
#else
               t1 = sm2x2avg_t2[0][0][0]+orient_distr*scat2x2m[0][0];
	       sm2x2avg_t2[0][0][0] = t1;
	       t2 = sm2x2avg_t2[0][1][0]+orient_distr*scat2x2m[0][1];
	       sm2x2avg_t2[0][1][0] = t1;
	       t3 = sm2x2avg_t2[1][0][0]+orient_distr*scat2x2m[1][0];
	       sm2x2avg_t2[1][0][0] = t3;
	       t4 = sm2x2avg_t2[1][1][0]+orient_distr*scat2x2m[1][1];
               sm2x2avg_t2[1][1][0] = t4;
#endif
