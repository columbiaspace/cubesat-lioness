#include "star_id.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>

double Vec3::Magnitude() const { return std::hypot(std::hypot(x, y), z); }
double Vec3::MagnitudeSq() const { return std::fma(x, x, std::fma(y, y, z * z)); }

Vec3 Vec3::Normalize() const {
    const double m = Magnitude();
    return {x / m, y / m, z / m};
}

double Vec3::operator*(const Vec3 &o) const { return std::fma(x, o.x, std::fma(y, o.y, z * o.z)); }
Vec3 Vec3::operator*(const double s) const { return {x * s, y * s, z * s}; }
Vec3 Vec3::operator-(const Vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }

Vec3 Vec3::CrossProduct(const Vec3 &o) const {
    return {y * o.z - z * o.y, -(x * o.z - z * o.x), x * o.y - y * o.x};
}

Mat3 Vec3::OuterProduct(const Vec3 &o) const {
    return {{x * o.x, x * o.y, x * o.z, y * o.x, y * o.y, y * o.z, z * o.x, z * o.y, z * o.z}};
}

Vec3 Vec3::operator*(const Mat3 &o) const {
    return {
        x * o.At(0, 0) + y * o.At(0, 1) + z * o.At(0, 2),
        x * o.At(1, 0) + y * o.At(1, 1) + z * o.At(1, 2),
        x * o.At(2, 0) + y * o.At(2, 1) + z * o.At(2, 2)
    };
}

double Mat3::At(uint8_t i, uint8_t j) const { return x[3 * i + j]; }

Mat3 Mat3::operator+(const Mat3 &o) const {
    return {
        {
            At(0, 0) + o.At(0, 0), At(0, 1) + o.At(0, 1), At(0, 2) + o.At(0, 2),
            At(1, 0) + o.At(1, 0), At(1, 1) + o.At(1, 1), At(1, 2) + o.At(1, 2),
            At(2, 0) + o.At(2, 0), At(2, 1) + o.At(2, 1), At(2, 2) + o.At(2, 2)
        }
    };
}

Mat3 Mat3::operator*(const Mat3 &o) const {
#define E(r,c) At(r,0)*o.At(0,c)+At(r,1)*o.At(1,c)+At(r,2)*o.At(2,c)
    return {{E(0, 0),E(0, 1),E(0, 2), E(1, 0),E(1, 1),E(1, 2), E(2, 0),E(2, 1),E(2, 2)}};
#undef E
}

Vec3 Mat3::operator*(const Vec3 &v) const {
    return {
        v.x * At(0, 0) + v.y * At(0, 1) + v.z * At(0, 2),
        v.x * At(1, 0) + v.y * At(1, 1) + v.z * At(1, 2),
        v.x * At(2, 0) + v.y * At(2, 1) + v.z * At(2, 2)
    };
}

Mat3 Mat3::operator*(double s) const {
    return {
        {
            s * At(0, 0), s * At(0, 1), s * At(0, 2),
            s * At(1, 0), s * At(1, 1), s * At(1, 2),
            s * At(2, 0), s * At(2, 1), s * At(2, 2)
        }
    };
}

Mat3 Mat3::Transpose() const {
    return {{At(0, 0), At(1, 0), At(2, 0), At(0, 1), At(1, 1), At(2, 1), At(0, 2), At(1, 2), At(2, 2)}};
}

double Mat3::Trace() const { return At(0, 0) + At(1, 1) + At(2, 2); }

double Mat3::Det() const {
    return At(0, 0) * (At(1, 1) * At(2, 2) - At(2, 1) * At(1, 2))
           - At(0, 1) * (At(1, 0) * At(2, 2) - At(2, 0) * At(1, 2))
           + At(0, 2) * (At(1, 0) * At(2, 1) - At(2, 0) * At(1, 1));
}

Mat3 Mat3::Inverse() const {
    return Mat3{
               {
                   At(1, 1) * At(2, 2) - At(1, 2) * At(2, 1),
                   At(0, 2) * At(2, 1) - At(0, 1) * At(2, 2),
                   At(0, 1) * At(1, 2) - At(0, 2) * At(1, 1),
                   At(1, 2) * At(2, 0) - At(1, 0) * At(2, 2),
                   At(0, 0) * At(2, 2) - At(0, 2) * At(2, 0),
                   At(0, 2) * At(1, 0) - At(0, 0) * At(1, 2),
                   At(1, 0) * At(2, 1) - At(1, 1) * At(2, 0),
                   At(0, 1) * At(2, 0) - At(0, 0) * At(2, 1),
                   At(0, 0) * At(1, 1) - At(0, 1) * At(1, 0)
               }
           }
           * (1.0 / Det());
}

const Mat3 kIdentityMat3 = {{1, 0, 0, 0, 1, 0, 0, 0, 1}};

EulerAngles Quaternion::ToSpherical() const {
    double ra = std::atan2(2 * (-real * k + i * j), 1 - 2 * (j * j + k * k));
    if (ra < 0) ra += 2 * M_PI;
    double de = -std::asin(std::clamp(2 * (-real * j - i * k), -1.0, 1.0));
    double roll = -std::atan2(2 * (-real * i + j * k), 1 - 2 * (i * i + j * j));
    if (roll < 0) roll += 2 * M_PI;
    return {ra, de, roll};
}

double Angle(const Vec3 &a, const Vec3 &b) {
    return AngleUnit(a.Normalize(), b.Normalize());
}

double AngleUnit(const Vec3 &a, const Vec3 &b) {
    const double d = a * b;
    return d >= 1 ? 0 : d <= -1 ? M_PI - 1e-7 : std::acos(d);
}

double RadToDeg(const double r) { return r * 180.0 / M_PI; }

static double DecimalModulo(const double x, const double mod) {
    const double r = x - mod * std::floor(x / mod);
    return r >= 0 ? r : r + mod;
}

Star::Star() : position({0, 0}), radiusX(0), radiusY(0), magnitude(0) {
}

Star::Star(const double x, const double y, const double rx, const double ry, const uint32_t mag)
    : position({x, y}), radiusX(rx), radiusY(ry), magnitude(mag) {
}

StarIdentifier::StarIdentifier(const int32_t si, const int16_t ci, const double w)
    : starIndex(si), catalogIndex(ci), weight(w) {
}

Camera::Camera(const double focalLength, const uint16_t xRes, const uint16_t yRes)
    : fl_(focalLength), cx_(xRes / 2.0), cy_(yRes / 2.0) {
}

Vec3 Camera::CameraToSpatial(const Vec2 &v) const {
    return {1.0, (-v.x + cx_) / fl_, (-v.y + cy_) / fl_};
}

class DeserializeContext {
public:
    explicit DeserializeContext(const uint8_t *buf) : buffer_(buf), cursor_(buf) {
    }

    [[nodiscard]] size_t GetOffset() const { return static_cast<size_t>(cursor_ - buffer_); }
    void MoveForward(const size_t n) { cursor_ += n; }
    [[nodiscard]] const uint8_t *GetCursor() const { return cursor_; }

private:
    const uint8_t *buffer_;
    const uint8_t *cursor_;
};

template<typename T>
static void star_tracker_deserialize_padding(DeserializeContext *des) {
    des->MoveForward((sizeof(T) - des->GetOffset() % sizeof(T)) % sizeof(T));
}

template<typename T>
static T star_tracker_deserialize_primitive(DeserializeContext *des) {
    star_tracker_deserialize_padding<T>(des);
    T result;
    std::memcpy(&result, des->GetCursor(), sizeof(T));
    des->MoveForward(sizeof(T));
    return result;
}

template<typename T>
static const T *star_tracker_deserialize_array(DeserializeContext *des, long len) {
    star_tracker_deserialize_padding<T>(des);
    const auto *result = reinterpret_cast<const T *>(des->GetCursor());
    des->MoveForward(sizeof(T) * len);
    return result;
}

static constexpr int32_t kCatalogMagicValue = static_cast<int32_t>(0xF9A283BC);

class MultiDatabase {
public:
    explicit MultiDatabase(const uint8_t *buf) : buffer_(buf) {
    }

    [[nodiscard]] const uint8_t *SubDatabasePointer(const int32_t magicValue) const {
        DeserializeContext des(buffer_);
        while (true) {
            auto cur = star_tracker_deserialize_primitive<int32_t>(&des);
            if (cur == 0) return nullptr;
            auto flags = star_tracker_deserialize_primitive<uint32_t>(&des);
            if (flags & 0x0001) {
                std::cerr << "[ERROR] Database built in float mode, expected double.\n";
                std::exit(1);
            }
            const auto len = star_tracker_deserialize_primitive<uint32_t>(&des);
            star_tracker_deserialize_padding<uint64_t>(&des);
            const auto *data = star_tracker_deserialize_array<uint8_t>(&des, len);
            if (cur == magicValue) return data;
        }
    }

private:
    const uint8_t *buffer_;
};

static Vec3 star_tracker_deserialize_vec3(DeserializeContext *des) {
    const auto x = star_tracker_deserialize_primitive<double>(des);
    const auto y = star_tracker_deserialize_primitive<double>(des);
    const auto z = star_tracker_deserialize_primitive<double>(des);
    return {x, y, z};
}

static Catalog star_tracker_deserialize_catalog(DeserializeContext *des) {
    const auto numStars = star_tracker_deserialize_primitive<int16_t>(des);
    if (numStars < 0) {
        std::cerr << "[ERROR] Invalid catalog size: " << numStars << "\n";
        std::exit(1);
    }

    const auto flags = star_tracker_deserialize_primitive<int8_t>(des);
    const bool inclMag = flags & 1;
    const bool inclName = (flags >> 1) & 1;
    Catalog result;
    result.reserve(numStars);
    for (int16_t i = 0; i < numStars; i++) {
        CatalogStar s;
        s.spatial = star_tracker_deserialize_vec3(des);
        if (inclMag) star_tracker_deserialize_primitive<double>(des);
        s.name = inclName ? star_tracker_deserialize_primitive<int16_t>(des) : -1;
        result.push_back(s);
    }
    return result;
}

Catalog star_tracker_load_catalog(const uint8_t *dbData) {
    const MultiDatabase multiDb(dbData);
    const uint8_t *catBuf = multiDb.SubDatabasePointer(kCatalogMagicValue);
    if (!catBuf) {
        std::cerr << "[ERROR] No catalog in database.\n";
        std::exit(1);
    }
    DeserializeContext des(catBuf);
    return star_tracker_deserialize_catalog(&des);
}

class KVectorIndex {
public:
    explicit KVectorIndex(DeserializeContext *des) {
        numValues_ = star_tracker_deserialize_primitive<int32_t>(des);
        min_ = star_tracker_deserialize_primitive<double>(des);
        max_ = star_tracker_deserialize_primitive<double>(des);
        numBins_ = star_tracker_deserialize_primitive<int32_t>(des);
        binWidth_ = (max_ - min_) / numBins_;
        bins_ = star_tracker_deserialize_array<int32_t>(des, numBins_ + 1);
    }

    int32_t QueryLiberal(double minQ, double maxQ, int32_t *upperIdx) const {
        if (maxQ >= max_) maxQ = max_ - 1e-5;
        if (minQ <= min_) minQ = min_ + 1e-5;
        if (minQ > max_ || maxQ < min_) {
            *upperIdx = 0;
            return 0;
        }
        const int32_t lo = BinFor(minQ), hi = BinFor(maxQ);
        const int32_t lower = bins_[lo - 1];
        if (lower >= numValues_) return 0;
        *upperIdx = bins_[hi];
        return lower;
    }

    [[nodiscard]] int32_t NumValues() const { return numValues_; }
    [[nodiscard]] double Min() const { return min_; }
    [[nodiscard]] double Max() const { return max_; }

private:
    [[nodiscard]] int32_t BinFor(const double q) const {
        return std::clamp(static_cast<int32_t>(std::ceil((q - min_) / binWidth_)), 0, numBins_);
    }

    int32_t numValues_, numBins_;
    double min_, max_, binWidth_;
    const int32_t *bins_;
};

class PairDistanceKVectorDatabase {
public:
    static constexpr int32_t kMagicValue = 0x2536f009;

    explicit PairDistanceKVectorDatabase(DeserializeContext *des)
        : index_(des) { pairs_ = star_tracker_deserialize_array<int16_t>(des, 2 * index_.NumValues()); }

    const int16_t *FindPairsLiberal(const double minD, const double maxD, const int16_t **end) const {
        int32_t upper = -1;
        const int32_t lower = index_.QueryLiberal(minD, maxD, &upper);
        *end = &pairs_[upper * 2];
        return &pairs_[lower * 2];
    }

    const int16_t *FindPairsExact(const Catalog &catalog, const double minD, const double maxD, const int16_t **end) const {
        const double maxCos = std::cos(minD), minCos = std::cos(maxD);
        int32_t liberalUpper;
        int32_t liberalLower = index_.QueryLiberal(minD, maxD, &liberalUpper);
        while (liberalLower < liberalUpper &&
               catalog[pairs_[liberalLower * 2]].spatial * catalog[pairs_[liberalLower * 2 + 1]].spatial >= maxCos)
            liberalLower++;
        while (liberalLower < liberalUpper &&
               catalog[pairs_[(liberalUpper - 1) * 2]].spatial * catalog[pairs_[(liberalUpper - 1) * 2 + 1]].spatial <=
               minCos)
            liberalUpper--;
        *end = &pairs_[liberalUpper * 2];
        return &pairs_[liberalLower * 2];
    }

    [[nodiscard]] double MinDistance() const { return index_.Min(); }
    [[nodiscard]] double MaxDistance() const { return index_.Max(); }

private:
    KVectorIndex index_;
    const int16_t *pairs_;
};

static std::unordered_multimap<int16_t, int16_t>
PairDistanceQueryToMap(const int16_t *pairs, const int16_t *end) {
    std::unordered_multimap<int16_t, int16_t> result;
    for (const int16_t *p = pairs; p != end; p += 2) {
        result.emplace(p[0], p[1]);
        result.emplace(p[1], p[0]);
    }
    return result;
}

class PairDistanceInvolvingIterator {
public:
    PairDistanceInvolvingIterator(const int16_t *pairs, const int16_t *end, int16_t involving)
        : pairs_(pairs), end_(end), involving_(involving) {
        while (pairs_ != end_ && pairs_[0] != involving_ && pairs_[1] != involving_)
            pairs_ += 2;
    }

    [[nodiscard]] bool HasValue() const { return pairs_ != end_; }
    int16_t operator*() const { return pairs_[0] == involving_ ? pairs_[1] : pairs_[0]; }

    PairDistanceInvolvingIterator &operator++() {
        pairs_ += 2;
        while (pairs_ != end_ && pairs_[0] != involving_ && pairs_[1] != involving_)
            pairs_ += 2;
        return *this;
    }

private:
    const int16_t *pairs_, *end_;
    int16_t involving_;
};

static std::vector<int16_t>
IdentifyThirdStar(const PairDistanceKVectorDatabase &db, const Catalog &catalog,
                  const int16_t catIdx1, const int16_t catIdx2,
                  const double dist1, const double dist2, const double tolerance) {
    const int16_t *q1End;
    const int16_t *q1 = db.FindPairsExact(catalog, dist1 - tolerance, dist1 + tolerance, &q1End);
    const Vec3 cross = catalog[catIdx1].spatial.CrossProduct(catalog[catIdx2].spatial);
    std::vector<int16_t> result;
    for (PairDistanceInvolvingIterator it(q1, q1End, catIdx1); it.HasValue(); ++it) {
        Vec3 cand = catalog[*it].spatial;
        const double a2 = AngleUnit(cand, catalog[catIdx2].spatial);
        if (a2 < dist2 - tolerance || a2 > dist2 + tolerance) continue;
        if (cross * cand <= 0) continue;
        result.push_back(*it);
    }
    return result;
}

class IRUnidentifiedCentroid {
public:
    double bestAngleFrom90;
    StarIdentifier bestStar1, bestStar2;
    int32_t index;
    const Star *star;

    IRUnidentifiedCentroid(const Star &s, const int32_t idx)
        : bestAngleFrom90(std::numeric_limits<double>::max()),
          bestStar1(0, 0), bestStar2(0, 0), index(idx), star(&s) {
    }

    void AddIdentifiedStar(const StarIdentifier &starId, const Stars &stars) {
        const Vec2 diff = stars[starId.starIndex].position - star->position;
        double angle = std::atan2(diff.y, diff.x);
        for (const auto &[otherAngle, otherId]: identifiedStarsInRange_) {
            const double a90 = std::abs(DecimalModulo(otherAngle - angle, M_PI) - M_PI_2);
            if (a90 < bestAngleFrom90) {
                bestAngleFrom90 = a90;
                bestStar1 = starId;
                bestStar2 = otherId;
            }
        }
        identifiedStarsInRange_.emplace_back(angle, starId);
    }

private:
    std::vector<std::pair<double, StarIdentifier> > identifiedStarsInRange_;
};

static std::vector<std::vector<IRUnidentifiedCentroid *>::iterator>
FindUnidentifiedCentroidsInRange(std::vector<IRUnidentifiedCentroid *> *centroids,
                                 const Star &star, const Camera &camera,
                                 const double minDist, const double maxDist) {
    const Vec3 ourSpatial = camera.CameraToSpatial(star.position).Normalize();
    const double minCos = std::cos(maxDist), maxCos = std::cos(minDist);
    std::vector<std::vector<IRUnidentifiedCentroid *>::iterator> result;
    for (auto it = centroids->begin(); it != centroids->end(); ++it) {
        const double c = ourSpatial * camera.CameraToSpatial((*it)->star->position).Normalize();
        if (c >= minCos && c <= maxCos) result.push_back(it);
    }
    return result;
}

static void AddToAllUnidentifiedCentroids(
    const StarIdentifier &starId, const Stars &stars,
    std::vector<IRUnidentifiedCentroid *> *above,
    std::vector<IRUnidentifiedCentroid *> *below,
    const double minDist, const double maxDist, const double threshold, const Camera &camera) {
    std::vector<int32_t> nowBelow;
    for (auto it: FindUnidentifiedCentroidsInRange(above, stars[starId.starIndex], camera, minDist, maxDist)) {
        (*it)->AddIdentifiedStar(starId, stars);
        if ((*it)->bestAngleFrom90 <= threshold) {
            below->push_back(*it);
            nowBelow.push_back((*it)->index);
        }
    }
    std::erase_if(*above,
                  [&](const IRUnidentifiedCentroid *c) {
                      return std::ranges::find(nowBelow, c->index) != nowBelow.end();
                  });
}

static IRUnidentifiedCentroid *SelectNextUnidentifiedCentroid(
    std::vector<IRUnidentifiedCentroid *> *above,
    std::vector<IRUnidentifiedCentroid *> *below) {
    if (!below->empty()) {
        const auto r = below->back();
        below->pop_back();
        return r;
    }
    const auto best = std::ranges::min_element(*above,
                                               [](const IRUnidentifiedCentroid *a, const IRUnidentifiedCentroid *b) {
                                                   return a->bestAngleFrom90 < b->bestAngleFrom90;
                                               });
    if (best != above->end() && (*best)->bestAngleFrom90 < 10) {
        const auto r = *best;
        above->erase(best);
        return r;
    }
    return nullptr;
}

static int32_t IdentifyRemainingStarsPairDistance(
    StarIdentifiers *identifiers, const Stars &stars,
    const PairDistanceKVectorDatabase &db, const Catalog &catalog,
    const Camera &camera, const double tolerance) {
    std::vector<IRUnidentifiedCentroid> all;
    std::vector<IRUnidentifiedCentroid *> above, below;
    all.reserve(stars.size());
    for (size_t i = 0; i < stars.size(); i++) {
        all.emplace_back(stars[i], static_cast<int32_t>(i));
    }

    above.reserve(all.size());
    for (auto &c: all) {
        bool found = false;
        for (const auto &id: *identifiers)
            if (id.starIndex == c.index) {
                found = true;
                break;
            }
        if (!found) above.push_back(&c);
    }

    for (const auto &id: *identifiers)
        AddToAllUnidentifiedCentroids(id, stars, &above, &below,
                                      db.MinDistance(), db.MaxDistance(), M_PI_4, camera);

    int32_t extra = 0;
    while (!below.empty() || !above.empty()) {
        auto *next = SelectNextUnidentifiedCentroid(&above, &below);
        if (!next) break;

        Vec3 unSp = camera.CameraToSpatial(next->star->position);
        Vec3 sp1 = camera.CameraToSpatial(stars[next->bestStar1.starIndex].position);
        Vec3 sp2 = camera.CameraToSpatial(stars[next->bestStar2.starIndex].position);
        const double d1 = Angle(sp1, unSp), d2 = Angle(sp2, unSp);
        const double spectral = sp1.CrossProduct(sp2) * unSp;

        auto candidates = spectral > 0
                              ? IdentifyThirdStar(db, catalog, next->bestStar1.catalogIndex,
                                                  next->bestStar2.catalogIndex, d1, d2, tolerance)
                              : IdentifyThirdStar(db, catalog, next->bestStar2.catalogIndex,
                                                  next->bestStar1.catalogIndex, d2, d1, tolerance);

        if (candidates.size() == 1) {
            identifiers->emplace_back(next->index, candidates[0]);
            AddToAllUnidentifiedCentroids(identifiers->back(), stars, &above, &below,
                                          db.MinDistance(), db.MaxDistance(), M_PI_4, camera);
            ++extra;
        }
    }
    return extra;
}

StarIdentifiers star_tracker_pyramid_star_id(
    const uint8_t *database, const Stars &stars, const Catalog &catalog,
    const Camera &camera, double tolerance, uint32_t numFalseStars,
    double maxMismatchProbability, uint64_t cutoff) {
    StarIdentifiers identified;
    MultiDatabase multiDb(database);
    const uint8_t *dbBuf = multiDb.SubDatabasePointer(PairDistanceKVectorDatabase::kMagicValue);
    if (!dbBuf || stars.size() < 4) {
        std::cerr << "[WARN] Not enough stars or database missing.\n";
        return identified;
    }
    DeserializeContext des(dbBuf);
    PairDistanceKVectorDatabase db(&des);

    double mc = std::pow(numFalseStars, 4) * std::pow(tolerance, 5) / 2.0 / (M_PI * M_PI);
    int32_t n = static_cast<int32_t>(stars.size());
    int32_t across = static_cast<int32_t>(std::floor(std::sqrt(n))) * 2;
    int32_t half = static_cast<int32_t>(std::floor(std::sqrt(n) / 2));
    uint64_t iters = 0;

    for (int32_t jI = 0; jI < n - 3; jI++) {
        int32_t dj = 1 + (jI + half) % (n - 3);
        for (int32_t kI = 0; kI < n - dj - 2; kI++) {
            int32_t dk = 1 + (kI + across) % (n - dj - 2);
            for (int32_t rI = 0; rI < n - dj - dk - 1; rI++) {
                int32_t dr = 1 + (rI + half) % (n - dj - dk - 1);
                int32_t iMax = n - dj - dk - dr - 1;
                for (int32_t iI = 0; iI <= iMax; iI++) {
                    int32_t i = (iI + iMax / 2) % (iMax + 1);
                    if (++iters > cutoff) {
                        std::cerr << "[WARN] Cutoff reached.\n";
                        return identified;
                    }

                    int32_t j = i + dj, k = j + dk, r = k + dr;
                    Vec3 iSp = camera.CameraToSpatial(stars[i].position).Normalize();
                    Vec3 jSp = camera.CameraToSpatial(stars[j].position).Normalize();
                    Vec3 kSp = camera.CameraToSpatial(stars[k].position).Normalize();

                    double ijDist = AngleUnit(iSp, jSp);
                    double iSin = std::sin(Angle(jSp - iSp, kSp - iSp));
                    double jSin = std::sin(Angle(iSp - jSp, kSp - jSp));
                    double kSin = std::sin(Angle(iSp - kSp, jSp - kSp));

                    double em = mc * std::sin(ijDist) / kSin / std::max({iSin, jSin, kSin});
                    if (em > maxMismatchProbability) continue;

                    Vec3 rSp = camera.CameraToSpatial(stars[r].position).Normalize();
                    bool spectral = iSp.CrossProduct(jSp) * kSp > 0;

                    double ikDist = AngleUnit(iSp, kSp);
                    double irDist = AngleUnit(iSp, rSp);
                    double jkDist = AngleUnit(jSp, kSp);
                    double jrDist = AngleUnit(jSp, rSp);
                    double krDist = AngleUnit(kSp, rSp);

#define CHK(d) if ((d)<db.MinDistance()+tolerance||(d)>db.MaxDistance()-tolerance) continue
                    CHK(ikDist);
                    CHK(irDist);
                    CHK(jkDist);
                    CHK(jrDist);
                    CHK(krDist);
#undef CHK

                    const int16_t *ijEnd, *ikEnd, *irEnd;
                    const int16_t *ijQ = db.FindPairsLiberal(ijDist - tolerance, ijDist + tolerance, &ijEnd);
                    const int16_t *ikQ = db.FindPairsLiberal(ikDist - tolerance, ikDist + tolerance, &ikEnd);
                    const int16_t *irQ = db.FindPairsLiberal(irDist - tolerance, irDist + tolerance, &irEnd);
                    auto ikMap = PairDistanceQueryToMap(ikQ, ikEnd);
                    auto irMap = PairDistanceQueryToMap(irQ, irEnd);

                    int16_t iM = -1, jM = -1, kM = -1, rM = -1;
                    for (const int16_t *q = ijQ; q != ijEnd; q++) {
                        int16_t iC = *q, jC = (q - ijQ) % 2 == 0 ? q[1] : q[-1];
                        Vec3 ijCross = catalog[iC].spatial.CrossProduct(catalog[jC].spatial);

                        for (auto kIt = ikMap.equal_range(iC); kIt.first != kIt.second; ++kIt.first) {
                            int16_t kC = kIt.first->second;
                            if ((ijCross * catalog[kC].spatial > 0) != spectral) continue;
                            double jkC = AngleUnit(catalog[jC].spatial, catalog[kC].spatial);
                            if (jkC < jkDist - tolerance || jkC > jkDist + tolerance) continue;

                            for (auto rIt = irMap.equal_range(iC); rIt.first != rIt.second; ++rIt.first) {
                                int16_t rC = rIt.first->second;
                                double jrC = AngleUnit(catalog[jC].spatial, catalog[rC].spatial);
                                if (jrC < jrDist - tolerance || jrC > jrDist + tolerance) continue;
                                double krC = AngleUnit(catalog[kC].spatial, catalog[rC].spatial);
                                if (krC < krDist - tolerance || krC > krDist + tolerance) continue;
                                if (iM == -1) {
                                    iM = iC;
                                    jM = jC;
                                    kM = kC;
                                    rM = rC;
                                } else {
                                    std::cerr << "[WARN] Pyramid not unique.\n";
                                    goto next;
                                }
                            }
                        }
                    }
                    if (iM != -1) {
                        std::cout << "[INFO] Matched unique pyramid.\n";
                        identified.emplace_back(i, iM);
                        identified.emplace_back(j, jM);
                        identified.emplace_back(k, kM);
                        identified.emplace_back(r, rM);
                        int32_t extra = IdentifyRemainingStarsPairDistance(&identified, stars, db, catalog, camera,
                                                                       tolerance);
                        std::cout << "[INFO] Identified " << extra << " additional stars.\n";
                        return identified;
                    }
                next:;
                }
            }
        }
    }
    std::cerr << "[WARN] Tried all pyramids; none matched.\n";
    return identified;
}

static double QuestCharPoly(const double x, const double a, const double b, const double c, const double d, const double s) {
    return (x * x - a) * (x * x - b) - c * x + c * s - d;
}

static double QuestCharPolyPrime(const double x, const double a, const double b, const double c) {
    return 4 * x * x * x - 2 * (a + b) * x - c;
}

static double QuestEigenvalue(double guess, const double a, const double b, const double c, const double d, const double s) {
    double h;
    do {
        h = QuestCharPoly(guess, a, b, c, d, s) / QuestCharPolyPrime(guess, a, b, c);
        guess -= h;
    } while (std::abs(h) >= 1e-4);
    return guess;
}

Quaternion star_tracker_quest_attitude(const Camera &camera, const Stars &stars,
                                       const Catalog &catalog, const StarIdentifiers &ids) {
    if (ids.size() < 2) return {1, 0, 0, 0};

    double guess = 0;
    Mat3 B = {{0, 0, 0, 0, 0, 0, 0, 0, 0}};
    for (const auto &s: ids) {
        Vec3 bi = camera.CameraToSpatial(stars[s.starIndex].position).Normalize();
        Vec3 ri = catalog[s.catalogIndex].spatial;
        B = B + (ri.OuterProduct(bi) * s.weight);
        guess += s.weight;
    }

    const Mat3 S = B + B.Transpose();
    const double sigma = B.Trace();
    const Vec3 Z = {B.At(1, 2) - B.At(2, 1), B.At(2, 0) - B.At(0, 2), B.At(0, 1) - B.At(1, 0)};

    const double delta = S.Det();
    const double kappa = (S.Inverse() * delta).Trace();
    const double a = sigma * sigma - kappa;
    const double b = sigma * sigma + (Z * Z);
    const double c = delta + (Z * S * Z);
    const double d = Z * (S * S) * Z;

    const double eig = QuestEigenvalue(guess, a, b, c, d, sigma);
    const double alpha = eig * eig - sigma * sigma + kappa;
    const double beta = eig - sigma;
    double gamma = (eig + sigma) * alpha - delta;

    Vec3 X = ((kIdentityMat3 * alpha) + (S * beta) + (S * S)) * Z;
    const double sc = 1.0 / std::sqrt(gamma * gamma + X.MagnitudeSq());
    X = X * sc;
    gamma *= sc;

    return {gamma, X.x, X.y, X.z};
}
