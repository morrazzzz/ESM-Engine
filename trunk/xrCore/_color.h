#pragma once

// maps unsigned 8 bits/channel to D3DCOLOR
ICF u32	color_argb	(u32 a, u32 r, u32 g, u32 b)	{	return ((a&0xff)<<24)|((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);	}
ICF u32	color_rgba	(u32 r, u32 g, u32 b, u32 a)	{	return color_argb(a,r,g,b);		}
ICF	u32	color_argb_f(f32 a, f32 r, f32 g, f32 b)	
{
	s32	 _r = clampr(iFloor(r*255.f),0,255);
	s32	 _g = clampr(iFloor(g*255.f),0,255);
	s32	 _b = clampr(iFloor(b*255.f),0,255);
	s32	 _a = clampr(iFloor(a*255.f),0,255);
	return color_argb(_a,_r,_g,_b);
}
ICF u32	color_rgba_f(f32 r, f32 g, f32 b, f32 a)	{	return color_argb_f(a,r,g,b);	}
ICF u32	color_xrgb	(u32 r, u32 g, u32 b)			{	return color_argb(0xff,r,g,b);	}
ICF	u32	color_get_R	(u32 rgba)						{	return (((rgba) >> 16) & 0xff);	}
ICF	u32	color_get_G	(u32 rgba)						{	return (((rgba) >> 8) & 0xff);	}
ICF	u32	color_get_B	(u32 rgba)						{	return ((rgba) & 0xff);			}
ICF	u32 color_get_A (u32 rgba)						{	return ((rgba) >> 24);			}
ICF u32 subst_alpha	(u32 rgba, u32 a)				{	return rgba&~color_rgba(0,0,0,0xff)|color_rgba(0,0,0,a);}
ICF u32 bgr2rgb		(u32 bgr)						{	return color_rgba(color_get_B(bgr),color_get_G(bgr),color_get_R(bgr),0);}
ICF u32 rgb2bgr		(u32 rgb)						{	return bgr2rgb(rgb);}

template <class T>
struct _color
{
	T r, g, b, a;

	ICF	_color& set(u32 dw)
	{
		const T f = T(1.0) / T(255.0);
		a = f * T((dw >> 24) & 0xff);
		r = f * T((dw >> 16) & 0xff);
		g = f * T((dw >> 8) & 0xff);
		b = f * T((dw >> 0) & 0xff);
		return *this;
	};
	IC	_color& set(T _r, T _g, T _b, T _a)
	{
		r = _r; g = _g; b = _b; a = _a;
		return *this;
	};

	IC _color& set(const _color& in)
	{
		r = in.r;
		g = in.g;
		b = in.b;
		a = in.a;
		return *this;
	};

	ICF	u32	get() const { return color_rgba_f(r, g, b, a); }

	IC _color& negative(const _color& in)
	{
		r = 1.0f - in.r;
		g = 1.0f - in.g;
		b = 1.0f - in.b;
		a = 1.0f - in.a;
		return *this;
	};

	IC _color& negative()
	{
		r = 1.0f - r;
		g = 1.0f - g;
		b = 1.0f - b;
		a = 1.0f - a;
		return *this;
	};

	IC _color& mul_rgba(T s)
	{
		r *= s;
		g *= s;
		b *= s;
		a *= s;
		return *this;
	};

	IC _color& mul_rgb(T s)
	{
		r *= s;
		g *= s;
		b *= s;
		return *this;
	};

	IC _color& mul_rgba(const _color& c, T s)
	{
		r = c.r * s;
		g = c.g * s;
		b = c.b * s;
		a = c.a * s;
		return *this;
	};

	IC _color& mul_rgb(const _color& c, T s)
	{
		r = c.r * s;
		g = c.g * s;
		b = c.b * s;
		return *this;
	};

	// SQ magnitude
	IC T magnitude_sqr_rgb() const {
		return r * r + g * g + b * b;
	}

	// magnitude
	IC T magnitude_rgb() const {
		return _sqrt(magnitude_sqr_rgb());
	}

	IC T intensity() const {
		return (r + g + b) / 3.f;
	}

	IC _color& lerp(const _color& c1, const _color& c2, T t)
	{
		T invt = 1.f - t;
		r = c1.r * invt + c2.r * t;
		g = c1.g * invt + c2.g * t;
		b = c1.b * invt + c2.b * t;
		a = c1.a * invt + c2.a * t;
		return *this;
	}

	IC _color& lerp(const _color& c1, const _color& c2, const _color& c3, T t)
	{
		if (t > .5f) {
			return lerp(c2, c3, t * 2.f - 1.f);
		}
		else {
			return lerp(c1, c2, t * 2.f);
		}
	}

	IC bool similar_rgba(const _color& v, T E = EPS_L) const { return _abs(r - v.r) < E && _abs(g - v.g) < E && _abs(b - v.b) < E && _abs(a - v.a) < E; }
	IC bool	similar_rgb(const _color& v, T E = EPS_L) const { return _abs(r - v.r) < E && _abs(g - v.g) < E && _abs(b - v.b) < E; }
};

using Fcolor = _color<float>;

template <class T>
BOOL	_valid			(const _color<T>& c)	{ return _valid(c.r) && _valid(c.g) && _valid(c.b) && _valid(c.a); }
