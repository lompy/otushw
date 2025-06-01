#include <iostream>
#include <limits>
#include <cmath>
#include <numeric>
#include <vector>
#include <algorithm>

class IStatistics {
public:
    virtual ~IStatistics() {}

    virtual void update(double next) = 0;
    virtual double eval() const = 0;
    virtual const char * name() const = 0;
};

class Min : public IStatistics {
public:
    Min() : min {std::numeric_limits<double>::max()} {}

    void update(double next) override {
        if (next < min)
            min = next;
    }

    double eval() const override {
        return min;
    }

    const char * name() const override {
        return "min";
    }

private:
    double min;
};

bool compare(const IStatistics& a, const IStatistics& b) {
    return a.eval() < b.eval();
}

class CompareAbsDiff {
public:
    CompareAbsDiff(double v) : val {v} {}
    bool operator()(const IStatistics &a, const IStatistics &b) {
        return std::abs(a.eval() - val) < std::abs(b.eval() - val);
    }

private:
    double val {};
};

class Max : public IStatistics {
public:
    Max() : max {std::numeric_limits<double>::min()} {}

    void update(double next) override {
        if (next > max)
            max = next;
    }

    double eval() const override {
        return max;
    }

    const char * name() const override {
        return "max";
    }

private:
    double max;
};

class Mean : public IStatistics {
public:
    Mean() = default;
    Mean(double m, int c = 1) : mean {m}, count {c} {}

    void update(double next) override {
        count++;
        mean += (next-mean) / count;
    }

    double eval() const override {
        return mean;
    }

    const char * name() const override {
        return "mean";
    }

    int get_count() const {
        return count;
    }
private:
    double mean {};
    int count {};
};

class Std : public IStatistics {
public:
    Std() = default;

    void update(double next) override {
        count++;
        auto prev_mean = mean.eval();
        mean.update(next);

        variance = (variance * (count-1) + (next - mean.eval()) * (next - prev_mean)) / count;
    }

    double eval() const override {
        return std::sqrt(variance);
    }

    const char * name() const override {
        return "std";
    }

private:
    int count;
    double variance {};
    Mean mean {};
};

class Quantile {
public:
    Quantile(size_t buf_size = 1024) {
        accums.reserve(buf_size);
        sorted_accums.reserve(buf_size);
    }

    void update(double next) {
        auto op = [&next](const Mean& acc) {
            return acc.eval() == next;
        };
        auto exact = std::find_if(accums.begin(), accums.end(), op);

        if (exact != accums.end())
            return exact->update(next);

        if (accums.size() < accums.capacity())
            accums.emplace_back(next);
        else
            std::min_element(
                accums.begin(),
                accums.end(),
                CompareAbsDiff(next)
            )->update(next);
    }

    double eval(double quantile) const {
        if (accums.empty())
            return std::numeric_limits<double>::quiet_NaN();
        if (accums.size() == 1)
            return accums[0].eval();

        double cutoff = quantile * total_count();
        double running_count = 0.0;

        sort();

        if (sorted_accums[0].get_count() > cutoff)
            return sorted_accums[0].eval();

        for (size_t i = 0; i < sorted_accums.size(); ++i) {
            double next_running_count = running_count + sorted_accums[i].get_count();

            if (next_running_count > cutoff) {
                double diff = (cutoff - running_count) / sorted_accums[i].get_count();

                return sorted_accums[i - 1].eval() * (1.0 - diff) + sorted_accums[i].eval() * diff;
            }

            running_count = next_running_count;
        }

        return sorted_accums.back().eval();
    }

private:
    std::vector<Mean> accums;
    mutable std::vector<Mean> sorted_accums;

    int total_count() const {
        auto op = [](int sum, const Mean& mean) {
            return sum + mean.get_count();
        };
        return std::accumulate(accums.begin(), accums.end(), 0, op);
    }

    void sort() const {
        sorted_accums.resize(accums.size());
        std::partial_sort_copy(
            accums.begin(),
            accums.end(),
            sorted_accums.begin(),
            sorted_accums.end(),
            compare);
    }
};

class Pct90 : public IStatistics {
public:
    void update(double next) override {
        quantile.update(next);
    }

    double eval() const override {
        return quantile.eval(0.90);
    }

    const char* name() const override {
        return "pct90";
    }

private:
    Quantile quantile;
};

class Pct95 : public IStatistics {
public:
    void update(double next) override {
        quantile.update(next);
    }

    double eval() const override {
        return quantile.eval(0.95);
    }

    const char* name() const override {
        return "pct95";
    }

private:
    Quantile quantile;
};

int main() {
    IStatistics* statistics[] = {
        new Min {},
        new Max {},
        new Mean {},
        new Std {},
        new Pct90 {},
        new Pct95 {},
    };

    double val {};
    while (std::cin >> val) {
        for (auto s: statistics)
            s->update(val);
    }

    if (!std::cin.eof() && !std::cin.good()) {
        std::cerr << "Invalid input data\n";
        return 1;
    }

    for (auto s: statistics)
        std::cout << s->name() << " = " << s->eval() << std::endl;

    for (auto s: statistics)
        delete s;

    return 0;
}
