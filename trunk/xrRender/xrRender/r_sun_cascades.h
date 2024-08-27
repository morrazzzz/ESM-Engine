#pragma once

namespace sun
{

	struct ray
	{
		ray() { D.set(0, 0, 0); P.set(0, 0, 0); }
		ray(Fvector3 const& _P, Fvector3 const& _D) : P(_P), D(_D) { }

		Fvector3 D;
		Fvector3 P;
	};

	struct cascade
	{
		cascade()
		{
			xform.identity();
			size = 0.0f;
			bias = 0.0f;
			reset_chain = false;
		}

		Fmatrix	xform;
		xr_vector<ray>	rays;
		float			size;
		float			bias;
		bool			reset_chain;
	};

} //namespace sun