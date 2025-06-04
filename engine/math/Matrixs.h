#pragma once

struct Matrix3x3 {
	float m[3][3];
};

struct Matrix4x4 {
	float m[4][4];

    // 行列同士の掛け算演算子オーバーロード
    Matrix4x4 operator*(const Matrix4x4& other) const
    {
        Matrix4x4 result;

        // 行列の掛け算
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }

        return result;
    }
};