#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>

class TDigest {
public:
    TDigest(size_t buf_size = 100) : max_centroids {buf_size} {}

    void add(double value) {
        if (centroids.empty()) {
            centroids.emplace_back(value, 1.0);
            return;
        }

        auto it = std::min_element(centroids.begin(), centroids.end(), [value](const Centroid& a, const Centroid& b) {
            return std::abs(a.mean - value) < std::abs(b.mean - value);
        });

        it->mean = (it->mean * it->count + value) / (it->count + 1);
        it->count += 1.0;

        total_count += 1.0;

        if (centroids.size() > max_centroids) {
            compress();
        }
    }

    double quantile(double q) {
        if (centroids.empty()) return std::numeric_limits<double>::quiet_NaN();
        if (centroids.size() == 1) return centroids[0].mean;

        std::sort(centroids.begin(), centroids.end(), [](const Centroid& a, const Centroid& b) {
            return a.mean < b.mean;
        });

        double rank = q * total_count;
        double cumulative = 0.0;

        for (size_t i = 0; i < centroids.size(); ++i) {
            double next_cumulative = cumulative + centroids[i].count;
            if (rank < next_cumulative) {
                if (i == 0) return centroids[i].mean;
                double r1 = cumulative;
                double r2 = next_cumulative;
                double m1 = centroids[i - 1].mean;
                double m2 = centroids[i].mean;
                double t = (rank - r1) / (r2 - r1);
                return m1 * (1.0 - t) + m2 * t;
            }
            cumulative = next_cumulative;
        }

        return centroids.back().mean;
    }

private:
    std::vector<Centroid> centroids;
    double total_count = 0.0;
    size_t max_centroids;

    void compress() {
        std::sort(centroids.begin(), centroids.end(), [](const Centroid& a, const Centroid& b) {
            return a.mean < b.mean;
        });

        std::vector<Centroid> compressed;
        for (const auto& c : centroids) {
            if (compressed.empty()) {
                compressed.push_back(c);
            } else {
                Centroid& last = compressed.back();
                if (compressed.size() < max_centroids) {
                    last.mean = (last.mean * last.count + c.mean * c.count) / (last.count + c.count);
                    last.count += c.count;
                } else {
                    compressed.push_back(c);
                }
            }
        }
        centroids = std::move(compressed);
    }
};
