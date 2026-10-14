// Compare all supported FFT sizes, including wrapping full-scale samples.
#include "fft.h"
#include <algorithm>
#include <cstdio>
#include <random>
void reference_fft_initialise();
void reference_fixed_fft(int16_t[], int16_t[], unsigned, bool);
void reference_fixed_ifft(int16_t[], int16_t[], unsigned);
int main() {
    fft_initialise(); reference_fft_initialise();
    std::mt19937 random(0x15f02026);
    unsigned cases=0;
    for(unsigned m=0;m<=8;++m) for(unsigned trial=0;trial<1000;++trial) {
        int16_t re[256]{}, im[256]{}, ref_re[256]{}, ref_im[256]{};
        const unsigned n=1u<<m;
        for(unsigned i=0;i<n;++i) {
            re[i]=trial==0?0:trial==1?32767:trial==2?-32768:static_cast<int16_t>(random());
            im[i]=trial<3?re[i]:static_cast<int16_t>(random());
        }
        std::copy(re,re+n,ref_re); std::copy(im,im+n,ref_im);
        fixed_fft(re,im,m); reference_fixed_fft(ref_re,ref_im,m,true);
        if(!std::equal(re,re+n,ref_re)||!std::equal(im,im+n,ref_im)) return 1;
        fixed_ifft(re,im,m); reference_fixed_ifft(ref_re,ref_im,m);
        if(!std::equal(re,re+n,ref_re)||!std::equal(im,im+n,ref_im)) return 2;
        ++cases;
    }
    std::printf("PASS %u FFT/IFFT cases: sizes 1..256, silence, rails and seeded full-range random input, bit-identical\n",cases);
}
