
#ifndef __GPIXEL_H
#define __GPIXEL_H 1

#include <stdint.h>
#include <math.h>

#define GIMEX_FORCE_INLINE __forceinline
#define GIMEX_INLINE __inline


const float b2f= 1 / 255.0f;
const int f2b = 255;

#define GF2B(v) v * f2b
#define GB2F(v) v * b2f

namespace gimex {
	namespace pixel {

		template <class T>
		class PixelTemplate
		{
		public:
			//enum { NumberOfElement = 4 };
			//typedef const PixelTemplate& InParam;

			// Constructors
			GIMEX_FORCE_INLINE PixelTemplate()
			{
				this->a = static_cast<T>(1);
				this->r = static_cast<T>(0);
				this->g = static_cast<T>(0);
				this->b = static_cast<T>(0);
			}
			/*
			GIMEX_FORCE_INLINE PixelTemplate(const PixelTemplate<T> &v)
			{
			this->a = static_cast<T>(v.a);
			this->r = static_cast<T>(v.r);
			this->g = static_cast<T>(v.g);
			this->b = static_cast<T>(v.b);
			}*/

			GIMEX_FORCE_INLINE PixelTemplate(const PixelTemplate<float> &v)
			{
				this->a = static_cast<T>(GF2B(v.a));
				this->r = static_cast<T>(GF2B(v.r));
				this->g = static_cast<T>(GF2B(v.g));
				this->b = static_cast<T>(GF2B(v.b));
			}

			GIMEX_FORCE_INLINE PixelTemplate(const PixelTemplate<uint8_t> &v)
			{
				this->a = static_cast<T>(GB2F(v.a));
				this->r = static_cast<T>(GB2F(v.r));
				this->g = static_cast<T>(GB2F(v.g));
				this->b = static_cast<T>(GB2F(v.b));
			}

			GIMEX_FORCE_INLINE explicit PixelTemplate(T scalar)
			{
				this->a = scalar;
				this->r = scalar;
				this->g = scalar;
				this->b = scalar;
			}

			GIMEX_FORCE_INLINE explicit PixelTemplate(T _a, T _r, T _g, T _b)
			{
				this->a = _a;
				this->r = _r;
				this->g = _g;
				this->b = _b;
			}

			// Operators
			
			GIMEX_FORCE_INLINE PixelTemplate& operator = (const PixelTemplate<T> &v)
			{
				this->x = v.x;
				this->y = v.y;
				this->z = v.z;
				this->w = v.w;
				return *this;
			}
			


			float Luminance()
			{
				return this.r * 0.299f + this.g * 0.587f + this.b * 0.114f;
			}

			float Gamma(float value)
			{
				this->r = pow(this->r,value);
				this->g = pow(this->g,value);
				this->b = pow(this->b,value);
				return this;
			}

		public:
			T a, r, g, b;

		};

		typedef PixelTemplate<uint8_t>   ARGBb;
		typedef PixelTemplate<float>     ARGBf;

	} // namespace pixel
} // namespace gimex

#endif //__GPIXEL_H