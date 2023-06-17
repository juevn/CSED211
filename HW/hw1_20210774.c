#include <stdio.h>
#include <limits.h>

typedef unsigned float_bits;

//1. Modified Problem of Exercise 2.61 on page 165. 
//1-A. The number of bits equal to 0 in the least significant byte (LSB) of x is two.
/* Least significant byte의 각 bit마다 0인지 확인하여
0이면 count에 1을 더해준 다음 최종적으로 count가 2인지 확인하여 return 한다. */
int lsb_two_check(int x)
{
	int n_0 = !(x & 1);
	int n_1 = !((x >> 1) & 1);
	int n_2 = !((x >> 2) & 1);
	int n_3 = !((x >> 3) & 1);
	int n_4 = !((x >> 4) & 1);
	int n_5 = !((x >> 5) & 1);
	int n_6 = !((x >> 6) & 1);
	int n_7 = !((x >> 7) & 1);

	int count = n_0 + n_1 + n_2 + n_3 + n_4 + n_5 + n_6 + n_7;

	int istrue = !(count - 2);
	return istrue;

}

//1-B.The number of bits equal to 1 in the least significant byte(LSB) of x is four.
/* Least significant byte의 각 bit마다1인지 확인하여
1이면 count에 1을 더해준 다음 최종적으로 count가 4인지 확인하여 return 한다. */
int lsb_four_check(int x)
{
	int n_0 = (x & 1);
	int n_1 = (x >> 1) & 1;
	int n_2 = (x >> 2) & 1;
	int n_3 = (x >> 3) & 1;
	int n_4 = (x >> 4) & 1;
	int n_5 = (x >> 5) & 1;
	int n_6 = (x >> 6) & 1;
	int n_7 = (x >> 7) & 1;

	int count = n_0 + n_1 + n_2 + n_3 + n_4 + n_5 + n_6 + n_7;

	int istrue = !(count - 4);
	return istrue;
}

//1-C. Only one bit in the most significant byte (MSB) of x equals 1. 
/* Most significant byte의 각 bit마다1인지 확인하여
1이면 count에 1을 더해준 다음 최종적으로 count가 1인지 확인하여 return 한다. */
int msb_check(int x)
{
	// msb를 lsb 자리로 옮긴 다음 구해준다.
	int msb = (x >> ((sizeof(int) - 1) << 3)) & 0XFF;

	int n_0 = (msb & 1);
	int n_1 = (msb >> 1) & 1;
	int n_2 = (msb >> 2) & 1;
	int n_3 = (msb >> 3) & 1;
	int n_4 = (msb >> 4) & 1;
	int n_5 = (msb >> 5) & 1;
	int n_6 = (msb >> 6) & 1;
	int n_7 = (msb >> 7) & 1;

	int count = n_0 + n_1 + n_2 + n_3 + n_4 + n_5 + n_6 + n_7;

	int istrue = !(count - 1);
	return istrue;
}

//1-D.One bit in MSB of x equals 1 and one bit in LSB of x equals 0.
/* Most significant byte의 각 bit마다1인지 확인하여 1이면 m_count에 1을 더해주고,
Least significant byte의 각 bit마다 0인지 확인하여 0이면 l_count에 1을 더해주고
최종적으로 m_count와 l_count가 1인지 확인하여 return 한다. */
int msblsb(int x)
{
	int msb = (x >> ((sizeof(int) - 1) << 3)) & 0XFF;
	int m_n1 = (msb & 1);
	int m_n2 = (msb >> 1) & 1;
	int m_n3 = (msb >> 2) & 1;
	int m_n4 = (msb >> 3) & 1;
	int m_n5 = (msb >> 4) & 1;
	int m_n6 = (msb >> 5) & 1;
	int m_n7 = (msb >> 6) & 1;
	int m_n8 = (msb >> 7) & 1;

	int m_count = m_n1 + m_n2 + m_n3 + m_n4 + m_n5 + m_n6 + m_n7 + m_n8;


	int l_n1 = !(x & 1);
	int l_n2 = !((x >> 1) & 1);
	int l_n3 = !((x >> 2) & 1);
	int l_n4 = !((x >> 3) & 1);
	int l_n5 = !((x >> 4) & 1);
	int l_n6 = !((x >> 5) & 1);
	int l_n7 = !((x >> 6) & 1);
	int l_n8 = !((x >> 7) & 1);

	int l_count = l_n1 + l_n2 + l_n3 + l_n4 + l_n5 + l_n6 + l_n7 + l_n8;

	return !(m_count - 1) && !(l_count - 1);
}


// 2. Exercise 2.73 on page 170
/* x<0, y<0, x+y >0 과 x>0, y>0, x+y <0 의 경우에만 overflow */
int saturating_add(int x, int y)
{
	int result = x + y;
	int sign_check = INT_MIN; // msb만 1, 나머지 bit들은 0 -> 이걸로 sign을 체크할 것이다. 

	// 1이면 음수, 0이면 양수
	int x_sign = x & sign_check;
	int y_sign = y & sign_check;
	int xy_sign = result & sign_check;

	int neg_of = x_sign && y_sign && !xy_sign; // neg + neg = pos 인 경우 1
	int pos_of = !x_sign && !y_sign && xy_sign; // pos + pos = neg 인 경우 1

	//overflow된 경우에는 result를 수정해준다.
	pos_of && (result = INT_MAX);
	neg_of && (result = INT_MIN);

	return result;
}


// 9. Exercise 2.92 on page 177
/* Compute -f. If f is Nan, then return f. */
/* 먼저 f에서 sign, exp, frac에 해당하는 비트 패턴을 shift 연산을 통해 구한다.
이 후 exp와 frac bits을 체크하여 Nan인지 확인하고 Nan이면 f을 리턴하고 아닌 경우에는
sign bit만 not 연산으로 바꿔준 후 sign, exp, frac을 같이 출력한다. */
float_bits float_negate(float_bits f)
{
	unsigned sign = f >> 31;
	unsigned exp = (f >> 23) & 0xFF;
	unsigned frac = f & 0X7FFFFF;
	if ((exp == 0XFF) && (frac != 0)) // NaN check
	{
		return f;
	}
	else // Nan이 아닌 경우
	{
		return (~sign << 31) | (exp << 23) | frac;
	}
}

//10. Modified Problem of Exercise 2.95 on page 178
/* Compute 0.25 * f. If f is Nan, then return f. */
float_bits float_half(float_bits f)
{
	unsigned sign = f >> 31;
	unsigned exp = (f >> 23) & 0xFF;
	unsigned frac = f & 0X7FFFFF;
	int roundup = 0;
	unsigned expfrac = f & 0X7FFFFFFF;

	if (exp == 0XFF) // NaN or infinite check
	{
		return f;
	}
	else // 나머지 경우
	{
		// 올림을 할지 체크하는 부분
		roundup = ((frac & 0X7) == 0x3) || ((frac & 0X7) == 0x6) || ((frac & 0X7) == 0x7);

		if (exp == 0) //frac을 shift right
		{
			frac >>= 2;
			frac += roundup;
		}
		else if ((exp == 1) || (exp == 2)) //denormalized로 바뀌는 경우
		{
			//exp part와 frac part 전체에서 1/2를 해준다.
			expfrac >>= 2;
			expfrac += roundup;

			exp = 0;
			//결과값으로 나온 expfrac을 shift를 통해 frac 파트로 다시 변환
			frac = expfrac & 0X7FFFFF;
		}
		else
		{
			exp -= 2;
		}

		return (sign << 31) | (exp << 23) | frac;
	}
}