#if !defined(CMATH_H)

union v2
{
    struct
    {
        f32 x, y;
    };
    struct
    {
        f32 X, Y;
    };
    struct
    {
        f32 u, v;
    };
    f32 E[2]; // Access elements as array
};

union intV2
{
    struct
    {
        u32 x, y;
    };
    struct
    {
        u32 x, y;
    };
    struct
    {
        u32 u, v;
    };
    u32 E[2]; // Access elements as array
};

inline intV2
INTV2(u32 X, u32 Y)
{
    intV2 Result;
    Result.x = X;
    Result.y = Y;
    return(Result);
}

inline v2
V2(f32 X, f32 Y)
{
    v2 Result;
    Result.x = X;
    Result.y = Y;
    return(Result);
}

inline v2
V2i(s32 X, s32 Y)
{
    // Convert signed types to floats.
    v2 Result = {(f32)X, (f32)Y};
    return(Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    // Convert unsigned types to floats
    v2 Result = {(f32)X, (f32)Y};
    return(Result);
}

// Addition
inline v2
operator+(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    return(Result);
}

inline v2 &
operator+=(v2 &B, v2 A)
{
    B = A + B;

    return(B);
}

// Subtraction
inline v2
operator-(v2 A, v2 B)
{
    v2 Result;
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    return(Result);
}

inline v2 &
operator-=(v2 &B, v2 A)
{
    B = A - B;

    return(B);
}

// Multiplication
inline v2
operator*(f32 A, v2 B)
{
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2
operator*(v2 A, f32 B)
{
    v2 Result;

    Result = B * A;
    
    return(Result);
}

inline v2 &
operator*=(v2 &B, f32 A)
{
    B = A * B;

    return(B);
}

// Negation
inline v2
operator-(v2 A)
{
    v2 Result;

    Result.x = -A.x;
    Result.y = -A.y;

    return(Result);
}

union v3
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 u, v, w;
    };
    struct
    {
        f32 r, g, b;
    };
    struct
    {
        v2 xy;
        f32 Ignored0_;
    };
    struct
    {
        f32 Ignored1_;
        v2 yz;
    };
    struct
    {
        v2 uv;
        f32 Ignored2_;
    };
    struct
    {
        f32 Ignored3_;
        v2 vw;
    };
    f32 E[3];
};

union rotator
{
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 pitch, yaw, roll;
    };
    f32 E[3];
};

inline rotator
operator+(rotator A, rotator B)
{
    rotator Result;

    Result.x = A.x+B.x;
    Result.y = A.y+B.y;
    Result.z = A.z+B.z;
    
    return(Result);
}

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                f32 x, y, z;
            };
        };
        
        f32 w;   
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                f32 r, g, b;
            };
        };
        
        f32 a;        
    };
    struct
    {
        v2 xy;
        f32 Ignored0_;
        f32 Ignored1_;
    };
    struct
    {
        f32 Ignored2_;
        v2 yz;
        f32 Ignored3_;
    };
    struct
    {
        f32 Ignored4_;
        f32 Ignored5_;
        v2 zw;
    };
    f32 E[4];
};

inline rotator
ROTATOR(f32 X, f32 Y, f32 Z)
{
    rotator Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return(Result);
}

inline v3
V3(f32 X, f32 Y, f32 Z)
{
    v3 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    return(Result);
}

inline f32
V3Length(v3 A)
{
    return sqrt((A.x * A.x) + (A.y * A.y) + (A.z * A.z));
}

inline v3
V3Normalize(v3 A)
{
    v3 Result;
    f32 Length = V3Length(A);
    Result.x = A.x / Length;
    Result.y = A.y / Length;
    Result.z = A.z / Length;
    return(Result);
}

inline v3
V3CrossProduct(v3 A, v3 B)
{
    v3 Result;
    Result.x = (A.y * B.z) - (A.z * B.y);
    Result.y = (A.z * B.x) - (A.x * B.z);
    Result.z = (A.x * B.y) - (A.y * B.x);
    return(Result);
}

inline v3
operator*(f32 A, v3 B)
{
    v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    
    return(Result);
}

inline v3
operator*(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x*B.x;
    Result.y = A.y*B.y;
    Result.z = A.z*B.z;
    
    return(Result);
}

inline v3
operator/(v3 A, f32 B)
{
    v3 Result;

    Result.x = A.x/B;
    Result.y = A.y/B;
    Result.z = A.z/B;
    
    return(Result);
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x+B.x;
    Result.y = A.y+B.y;
    Result.z = A.z+B.z;
    
    return(Result);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x-B.x;
    Result.y = A.y-B.y;
    Result.z = A.z-B.z;
    
    return(Result);
}

inline v3 &
operator+=(v3 &B, v3 A)
{
    B = A + B;
    return B;
}

inline v4
V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return(Result);
}

// Addition
inline v4
operator+(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

    return(Result);
}

inline v4 &
operator+=(v4 &B, v4 A)
{
    B = A + B;
    return B;
}

// Subtraction
inline v4
operator-(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;

    return(Result);
}

inline v4 &
operator-=(v4 &B, v4 A)
{
    B = A - B;
    return B;
}

// Multiplication

// TODO: Need v4 multiply

inline v4
operator*(f32 A, v4 B)
{
    v4 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;
    
    return(Result);
}

inline v4
operator*(v4 B, f32 A)
{
    v4 Result = A*B;

    return(Result);
}

inline v4 &
operator*=(v4 &B, f32 A)
{
    B = A * B;

    return(B);
}

// Negation
inline v4
operator-(v4 A)
{
    v4 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;

    return(Result);
}

union matrix3x3
{
    struct
    {
        f32 m11;
        f32 m12;
        f32 m13;

        f32 m21;
        f32 m22;
        f32 m23;

        f32 m31;
        f32 m32;
        f32 m33;
    };
    struct
    {
        v3 row1;
        v3 row2;
        v3 row3;
    };

    inline v3 col1()
    {
        return v3{m11, m21, m31};
    };

    inline v3 col2()
    {
        return v3{m12, m22, m32};
    };

    inline v3 col3()
    {
        return v3{m13, m23, m33};
    };
};

union matrix4x4
{
    struct
    {
        f32 m11;
        f32 m12;
        f32 m13;
        f32 m14;

        f32 m21;
        f32 m22;
        f32 m23;
        f32 m24;

        f32 m31;
        f32 m32;
        f32 m33;
        f32 m34;
        
        f32 m41;
        f32 m42;
        f32 m43;
        f32 m44;

    };
    struct
    {
        v4 row1;
        v4 row2;
        v4 row3;
        v4 row4;

    };

    inline v4 col1()
    {
        return v4{m11, m21, m31, m41};
    };

    inline v4 col2()
    {
        return v4{m12, m22, m32, m42};
    };

    inline v4 col3()
    {
        return v4{m13, m23, m33, m43};
    };
    
    inline v4 col4()
    {
        return v4{m14, m24, m34, m44};
    };

    matrix4x4& operator=(const DirectX::XMMATRIX& A);
    DirectX::XMMATRIX ToDXM();
};

inline v4
Lerp(v4 A, f32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline f32
DotProduct(v2 A, v2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

inline f32
DotProduct(v3 A, v3 B)
{
    f32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return(Result);
}

inline f32
DotProduct(v4 A, v4 B)
{
    f32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return(Result);
}

inline matrix3x3
operator*(matrix3x3 A, matrix3x3 B)
{
    matrix3x3 Result;

    // Row 1
    Result.m11 = DotProduct(A.row1, B.col1());
    Result.m12 = DotProduct(A.row1, B.col2());
    Result.m13 = DotProduct(A.row1, B.col3());

    // Row 2
    Result.m21 = DotProduct(A.row2, B.col1());
    Result.m22 = DotProduct(A.row2, B.col2());
    Result.m23 = DotProduct(A.row2, B.col3()); 

    // Row 3
    Result.m31 = DotProduct(A.row3, B.col1());
    Result.m32 = DotProduct(A.row3, B.col2());
    Result.m33 = DotProduct(A.row3, B.col3());

    return Result;
}

inline matrix4x4
operator*(matrix4x4 A, matrix4x4 B)
{
    matrix4x4 Result;

    // Row 1
    Result.m11 = DotProduct(A.row1, B.col1());
    Result.m12 = DotProduct(A.row1, B.col2());
    Result.m13 = DotProduct(A.row1, B.col3());
    Result.m14 = DotProduct(A.row1, B.col4());

    // Row 2
    Result.m21 = DotProduct(A.row2, B.col1());
    Result.m22 = DotProduct(A.row2, B.col2());
    Result.m23 = DotProduct(A.row2, B.col3()); 
    Result.m24 = DotProduct(A.row2, B.col4()); 

    // Row 3
    Result.m31 = DotProduct(A.row3, B.col1());
    Result.m32 = DotProduct(A.row3, B.col2());
    Result.m33 = DotProduct(A.row3, B.col3());
    Result.m34 = DotProduct(A.row3, B.col4());

    // Row 4
    Result.m41 = DotProduct(A.row4, B.col1());
    Result.m42 = DotProduct(A.row4, B.col2());
    Result.m43 = DotProduct(A.row4, B.col3());
    Result.m44 = DotProduct(A.row4, B.col4());

    return Result;
}

inline matrix4x4&
matrix4x4::operator=(const DirectX::XMMATRIX& A) // assumes transposition
{
    DirectX::XMFLOAT4X4 Res;
    DirectX::XMStoreFloat4x4(&Res, DirectX::XMMatrixTranspose(A));

    m11 = Res._11;
    m12 = Res._12;
    m13 = Res._13;
    m14 = Res._14;
    m21 = Res._21;
    m22 = Res._22;
    m23 = Res._23;
    m24 = Res._24;
    m31 = Res._31;
    m32 = Res._32;
    m33 = Res._33;
    m34 = Res._34;
    m41 = Res._41;
    m42 = Res._42;
    m43 = Res._43;
    m44 = Res._44;

    return *this;
}

inline DirectX::XMMATRIX
matrix4x4::ToDXM()
{
    DirectX::XMMATRIX Res;
    Res = DirectX::XMMatrixSet(
        m11,
        m12,
        m13,
        m14,
        m21,
        m22,
        m23,
        m24,
        m31,
        m32,
        m33,
        m34,
        m41,
        m42,
        m43,
        m44
    );

    return Res;
}

inline DirectX::XMMATRIX
M4ToDXM(matrix4x4 A)
{
    DirectX::XMMATRIX Res;
    Res = DirectX::XMMatrixSet(
        A.m11,
        A.m12,
        A.m13,
        A.m14,
        A.m21,
        A.m22,
        A.m23,
        A.m24,
        A.m31,
        A.m32,
        A.m33,
        A.m34,
        A.m41,
        A.m42,
        A.m43,
        A.m44
    );

    return Res;
}

// inline matrix4x4
// MATRIX4X4(f32 m11, f32 m12, f32 m13, f32 m14,
//           f32 m21, f32 m22, f32 m23, f32 m24,
//           f32 m31, f32 m32, f32 m33, f32 m34,
//           f32 m41, f32 m42, f32 m43, f32 m44)
// {
//     matrix4x4 Result;

//     // Row 1
//     Result.m11 = m11;
//     Result.m12 = m12;
//     Result.m13 = m13;
//     Result.m14 = m14;

//     // Row 2
//     Result.m21 = m21;
//     Result.m22 = m22;
//     Result.m23 = m23;
//     Result.m24 = m24;

//     // Row 3
//     Result.m31 = m25;
//     Result.m32 = DotProduct(A.row3, B.col2());
//     Result.m33 = DotProduct(A.row3, B.col3());
//     Result.m34 = DotProduct(A.row3, B.col4());

//     // Row 4
//     Result.m41 = DotProduct(A.row4, B.col1());
//     Result.m42 = DotProduct(A.row4, B.col2());
//     Result.m43 = DotProduct(A.row4, B.col3());
//     Result.m44 = DotProduct(A.row4, B.col4());

//     return Result;
// }

// bool PointInTriangle(DirectX::XMVECTOR triV1, DirectX::XMVECTOR triV2, DirectX::XMVECTOR triV3, DirectX::XMVECTOR point )
// {
//     To find out if the point is inside the triangle, we will check to see if the point
//     is on the correct side of each of the triangles edges.

//     DirectX::XMVECTOR cp1 = DirectX::XMVector3Cross((triV3 - triV2), (point - triV2));
//     DirectX::XMVECTOR cp2 = DirectX::XMVector3Cross((triV3 - triV2), (triV1 - triV2));
//     if(DirectX::XMVectorGetX(DirectX::XMVector3Dot(cp1, cp2)) >= 0)
//     {
//         cp1 = DirectX::XMVector3Cross((triV3 - triV1), (point - triV1));
//         cp2 = DirectX::XMVector3Cross((triV3 - triV1), (triV2 - triV1));
//         if(DirectX::XMVectorGetX(DirectX::XMVector3Dot(cp1, cp2)) >= 0)
//         {
//             cp1 = DirectX::XMVector3Cross((triV2 - triV1), (point - triV1));
//             cp2 = DirectX::XMVector3Cross((triV2 - triV1), (triV3 - triV1));
//             if(DirectX::XMVectorGetX(DirectX::XMVector3Dot(cp1, cp2)) >= 0)
//             {
//                 return true;
//             }
//             else
//                 return false;
//         }
//         else
//             return false;
//     }
//     return false;
// }

bool RayIntersectsPlane(DirectX::XMVECTOR Plane1, DirectX::XMVECTOR Plane2, DirectX::XMVECTOR RayOrigin, DirectX::XMVECTOR RayDirection )
{
    using namespace DirectX;
    //Find the normal using U, V coordinates (two edges)
    XMVECTOR U = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR V = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR faceNormal = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    U = Plane1;
    V = Plane2;

    //Compute face normal by crossing U, V
    faceNormal = XMVector3Cross(U, V);
    faceNormal = XMVector3Normalize(faceNormal);

    //Get plane equation ("Ax + By + Cz + D = 0") Variables
    float tri1A = XMVectorGetX(faceNormal);
    float tri1B = XMVectorGetY(faceNormal);
    float tri1C = XMVectorGetZ(faceNormal);
    float tri1D = (-tri1A*XMVectorGetX(Plane1) - tri1B*XMVectorGetY(Plane1) - tri1C*XMVectorGetZ(Plane1));

    //Now we find where (on the ray) the ray intersects with the triangles plane
    float ep1, ep2, t = 0.0f;
    //float planeIntersectX, planeIntersectY, planeIntersectZ = 0.0f;
    XMVECTOR pointInPlane = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    ep1 = (XMVectorGetX(RayOrigin) * tri1A) + (XMVectorGetY(RayOrigin) * tri1B) + (XMVectorGetZ(RayOrigin) * tri1C);
    ep2 = (XMVectorGetX(RayDirection) * tri1A) + (XMVectorGetY(RayDirection) * tri1B) + (XMVectorGetZ(RayDirection) * tri1C);

    //Make sure there are no divide-by-zeros
    if(ep2 != 0.0f)
        t = -(ep1 + tri1D)/(ep2);

    if (t > 0.0f)
    {
        // float Dot = XMVectorGetX(XMVector3Dot(RayDirection, RayDirection));
        // if (Dot <= 0.001f && Dot >= -0.001f)
        // {
        //     return true;
        // }
        // else
        //     return false;
        return true;
    }
    else
        return false;

}

struct colors
{
    static const v4 White;
    static const v4 Gray;
    static const v4 Black;
    static const v4 Translucent;
    static const v4 Red;
    static const v4 Green;
    static const v4 Blue;
    static const v4 Yellow;
    static const v4 Purple;
};

const v4 colors::White = V4(1.f, 1.f, 1.f, 1.f);
const v4 colors::Gray = V4(0.5f, 0.5f, 0.5f, 1.f);
const v4 colors::Black = V4(0.f, 0.f, 0.f, 1.f);
const v4 colors::Translucent = V4(1.f, 1.f, 1.f, 0.4f);
const v4 colors::Red = V4(1.f, 0.f, 0.f, 1.f);
const v4 colors::Green = V4(0.f, 1.f, 0.f, 1.f);
const v4 colors::Blue = V4(0.f, 0.f, 1.f, 1.f);
const v4 colors::Yellow = V4(1.f, 1.f, 0.f, 1.f);
const v4 colors::Purple = V4(1.f, 0.f, 1.f, 1.f);

#define CMATH_H
#endif