#include <stdio.h>
#include <limits.h>

typedef unsigned float_bits;

//1. Modified Problem of Exercise 2.61 on page 165. 
//1-A. The number of bits equal to 0 in the least significant byte (LSB) of x is two.
/* Least significant byte�� �� bit���� 0���� Ȯ���Ͽ�
0�̸� count�� 1�� ������ ���� ���������� count�� 2���� Ȯ���Ͽ� return �Ѵ�. */
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
/* Least significant byte�� �� bit����1���� Ȯ���Ͽ�
1�̸� count�� 1�� ������ ���� ���������� count�� 4���� Ȯ���Ͽ� return �Ѵ�. */
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
/* Most significant byte�� �� bit����1���� Ȯ���Ͽ�
1�̸� count�� 1�� ������ ���� ���������� count�� 1���� Ȯ���Ͽ� return �Ѵ�. */
int msb_check(int x)
{
	// msb�� lsb �ڸ��� �ű� ���� �����ش�.
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
/* Most significant byte�� �� bit����1���� Ȯ���Ͽ� 1�̸� m_count�� 1�� �����ְ�,
Least significant byte�� �� bit���� 0���� Ȯ���Ͽ� 0�̸� l_count�� 1�� �����ְ�
���������� m_count�� l_count�� 1���� Ȯ���Ͽ� return �Ѵ�. */
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
/* x<0, y<0, x+y >0 �� x>0, y>0, x+y <0 �� ��쿡�� overflow */
int saturating_add(int x, int y)
{
	int result = x + y;
	int sign_check = INT_MIN; // msb�� 1, ������ bit���� 0 -> �̰ɷ� sign�� üũ�� ���̴�. 

	// 1�̸� ����, 0�̸� ���
	int x_sign = x & sign_check;
	int y_sign = y & sign_check;
	int xy_sign = result & sign_check;

	int neg_of = x_sign && y_sign && !xy_sign; // neg + neg = pos �� ��� 1
	int pos_of = !x_sign && !y_sign && xy_sign; // pos + pos = neg �� ��� 1

	//overflow�� ��쿡�� result�� �������ش�.
	pos_of && (result = INT_MAX);
	neg_of && (result = INT_MIN);

	return result;
}


// 9. Exercise 2.92 on page 177
/* Compute -f. If f is Nan, then return f. */
/* ���� f���� sign, exp, frac�� �ش��ϴ� ��Ʈ ������ shift ������ ���� ���Ѵ�.
�� �� exp�� frac bits�� üũ�Ͽ� Nan���� Ȯ���ϰ� Nan�̸� f�� �����ϰ� �ƴ� ��쿡��
sign bit�� not �������� �ٲ��� �� sign, exp, frac�� ���� ����Ѵ�. */
float_bits float_negate(float_bits f)
{
	unsigned sign = f >> 31;
	unsigned exp = (f >> 23) & 0xFF;
	unsigned frac = f & 0X7FFFFF;
	if ((exp == 0XFF) && (frac != 0)) // NaN check
	{
		return f;
	}
	else // Nan�� �ƴ� ���
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
	else // ������ ���
	{
		// �ø��� ���� üũ�ϴ� �κ�
		roundup = ((frac & 0X7) == 0x3) || ((frac & 0X7) == 0x6) || ((frac & 0X7) == 0x7);

		if (exp == 0) //frac�� shift right
		{
			frac >>= 2;
			frac += roundup;
		}
		else if ((exp == 1) || (exp == 2)) //denormalized�� �ٲ�� ���
		{
			//exp part�� frac part ��ü���� 1/2�� ���ش�.
			expfrac >>= 2;
			expfrac += roundup;

			exp = 0;
			//��������� ���� expfrac�� shift�� ���� frac ��Ʈ�� �ٽ� ��ȯ
			frac = expfrac & 0X7FFFFF;
		}
		else
		{
			exp -= 2;
		}

		return (sign << 31) | (exp << 23) | frac;
	}
}