#ifndef STAR_ID_HPP
#define STAR_ID_HPP

#include <vector>

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
    [[nodiscard]] double At(int i, int j) const;
    Mat3 operator+(const Mat3 &o) const;
    Mat3 operator*(const Mat3 &o) const;
    Vec3 operator*(const Vec3 &v) const;
    Mat3 operator*(double s) const;
    [[nodiscard]] Mat3 Transpose() const;
    [[nodiscard]] double Trace() const;
    [[nodiscard]] double Det() const;
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
    double radiusX, radiusY;
    int magnitude;
    Star();
    Star(double x, double y, double rx, double ry, int mag);
};

struct CatalogStar {
    Vec3 spatial;
    int name;
};

struct StarIdentifier {
    int starIndex, catalogIndex;
    double weight;
    StarIdentifier(int si, int ci, double w = 1.0);
};

using Catalog = std::vector<CatalogStar>;
using Stars = std::vector<Star>;
using StarIdentifiers = std::vector<StarIdentifier>;

struct Camera {
    Camera(double focalLength, int xRes, int yRes);
    [[nodiscard]] Vec3 CameraToSpatial(const Vec2 &v) const;
private:
    double fl_, cx_, cy_;
};

Catalog star_tracker_load_catalog(const unsigned char *dbData);

StarIdentifiers star_tracker_pyramid_star_id(
    const unsigned char *database, const Stars &stars, const Catalog &catalog,
    const Camera &camera, double tolerance, int numFalseStars,
    double maxMismatchProbability, long cutoff);

Quaternion star_tracker_quest_attitude(const Camera &camera, const Stars &stars,
                         const Catalog &catalog, const StarIdentifiers &ids);

#endif
