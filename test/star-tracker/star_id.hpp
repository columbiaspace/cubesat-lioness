#ifndef STAR_ID_HPP
#define STAR_ID_HPP

#include <cstdint>

// Constants for numerical stability
constexpr double kEpsilon = 1e-15;
constexpr double kEpsilonSq = kEpsilon * kEpsilon;
constexpr double kQuestConvergenceTol = 1e-4;
constexpr int kQuestMaxIter = 50;

struct Vec2 {
    double x, y;
    Vec2 operator-(const Vec2 &o) const { return {x - o.x, y - o.y}; }
};

struct Mat3;

struct Vec3 {
    double x, y, z;
    [[nodiscard]] double Magnitude() const;
    [[nodiscard]] double MagnitudeSq() const;
    [[nodiscard]] Vec3 Normalize() const;
    double operator*(const Vec3 &o) const;
    Vec3 operator*(double s) const;
    Vec3 operator-(const Vec3 &o) const;
    [[nodiscard]] Vec3 CrossProduct(const Vec3 &o) const;
    [[nodiscard]] Mat3 OuterProduct(const Vec3 &o) const;
    Vec3 operator*(const Mat3 &m) const;
};

struct Mat3 {
    double x[9];
    [[nodiscard]] double At(uint8_t i, uint8_t j) const;
    Mat3 operator+(const Mat3 &o) const;
    Mat3 operator*(const Mat3 &o) const;
    Vec3 operator*(const Vec3 &v) const;
    Mat3 operator*(double s) const;
    [[nodiscard]] Mat3 Transpose() const;
    [[nodiscard]] double Trace() const;
    [[nodiscard]] double Det() const;
    [[nodiscard]] Mat3 Adjugate() const;
    [[nodiscard]] Mat3 Inverse() const;
};

extern const Mat3 kIdentityMat3;

struct EulerAngles { double ra, de, roll; };

struct Quaternion {
    double real, i, j, k;
    [[nodiscard]] EulerAngles ToSpherical() const;
};

double Angle(const Vec3 &a, const Vec3 &b);
double AngleUnit(const Vec3 &a, const Vec3 &b);
double RadToDeg(double r);

struct Star {
    Vec2 position;
    uint32_t magnitude;
    Star();
    Star(double x, double y, uint32_t mag);
};

struct CatalogStar {
    Vec3 spatial;
    int16_t name;
};

struct StarIdentifier {
    uint8_t starIndex;
    int16_t catalogIndex;
    double weight = 1.0;
};

constexpr uint8_t kMaxStars = 40;
constexpr uint16_t kMaxCatalogStars = 5000;

template <typename T, typename SizeT, SizeT MaxSize>
struct FixedArray {
    T data[MaxSize];
    SizeT count = 0;
    const T &operator[](SizeT i) const { return data[i]; }
    T &operator[](SizeT i) { return data[i]; }
    [[nodiscard]] SizeT size() const { return count; }
    [[nodiscard]] bool empty() const { return count == 0; }
    void push_back(const T &v) { data[count++] = v; }
    [[nodiscard]] const T &back() const { return data[count - 1]; }
    void pop_back() { --count; }
    [[nodiscard]] const T *begin() const { return data; }
    [[nodiscard]] const T *end() const { return data + count; }
    T *begin() { return data; }
    T *end() { return data + count; }
    T *erase(T *pos) {
        for (T *p = pos; p < data + count - 1; ++p)
            *p = *(p + 1);
        --count;
        return pos;
    }
    T *erase(T *first, T *last) {
        T *dst = first;
        for (T *src = last; src < data + count; ++src)
            *dst++ = *src;
        count -= static_cast<SizeT>(last - first);
        return first;
    }
};

using Stars = FixedArray<Star, uint8_t, kMaxStars>;
using Catalog = FixedArray<CatalogStar, uint16_t, kMaxCatalogStars>;

struct StarIdentifiers : FixedArray<StarIdentifier, uint8_t, kMaxStars> {
    void emplace_back(const uint8_t si, const int16_t ci, const double w = 1.0) { data[count++] = {si, ci, w}; }
};

struct Camera {
    Camera(double focalLength, uint16_t xRes, uint16_t yRes);
    [[nodiscard]] Vec3 CameraToSpatial(const Vec2 &v) const;
private:
    double fl_, cx_, cy_;
};

void star_tracker_load_catalog(const uint8_t *dbData, Catalog &catalog);

void star_tracker_pyramid_star_id(
    const uint8_t *database, const Stars &stars, const Catalog &catalog,
    const Camera &camera, StarIdentifiers &identified, double tolerance,
    uint32_t numFalseStars, double maxMismatchProbability, uint64_t cutoff);

Quaternion star_tracker_quest_attitude(const Camera &camera, const Stars &stars,
                         const Catalog &catalog, const StarIdentifiers &ids);

#endif
