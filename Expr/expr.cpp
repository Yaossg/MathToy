#include <map>
#include <set>
#include <string>
#include <iterator>
#include <numeric>
#include <cmath>
#include <compare>

#include "../yao_math.h"

#ifndef YAO_MATH_EXPR
#define YAO_MATH_EXPR

namespace yao_math { 

class Mono {
	std::map<std::string, size_t> core;
public:
	Mono(): core{} {}
    Mono(std::map<std::string, size_t> core): core{std::move(core)} {}

    friend bool operator ==(Mono const& a, Mono const& b) {
        return a.core == b.core;
    }

    friend std::strong_ordering operator <=>(Mono const& a, Mono const& b) {
        if (auto cmp = b.degree() <=> a.degree(); cmp != 0) return cmp;
        return a.core <=> b.core;
    }

	friend Mono operator *(Mono const& a, Mono const& b) {
		Mono r = a;
		for (auto const& [x, n] : b.core)
			r.core[x] += n;
		return r;
	}
	friend std::string toTex(Mono const& t) {
		std::string r;
		for (auto const& [x, n] : t.core) {
			r += x;
			if (n > 1) {
				r += "^";
				auto exp = toTex(n);
				if (exp.length() > 1) exp = "{" + exp + "}";
				r += exp;
			}
		}
		return r;
	}
	bool empty() const { return core.empty(); }
	void erase(std::string const& name) { core.erase(name); }
	bool contains(std::string const& name) const {
		return core.find(name) != core.end();
	}
	size_t at(std::string const& name) const {
		return core.at(name);
	}
	size_t degree() const {
		size_t m = 0;
		for (auto const& [x, n] : core)
			m = std::max(m, n);
		return m;
	}
};

template<typename CInt>
class RatioExpr;

template<typename CInt>
class IntExpr {
	friend class RatioExpr<CInt>;
	using mono = Mono;
	using core_t = std::map<mono, CInt>;
	core_t core;
    IntExpr(core_t core): core{std::move(core)} {}
	
	void div(CInt g) {
		for (auto& [x1, k1] : core) {
			k1 /= g;
		}
	}
public:
	using coefficient_t = CInt;
	IntExpr(CInt const& C = 0): core{{ {}, C }} {}
	IntExpr(std::string const& x, size_t n = 1): core{{ { {{x, n}} }, 1 }, {{},0}} {}
	IntExpr(mono const& mono, CInt const& C = 1): core{{ mono, C }, {{},0}} {}

	explicit operator bool() const {
		return !operator!();
	}

	bool operator!() const { return core == core_t{{{}, 0}}; }
	
	IntExpr operator+() const {
		return *this;
	}
	
	IntExpr operator-() const {
		return *this * -1;
	}
	
	IntExpr& operator+=(IntExpr const& other) {
		for (auto const& [x1, k1] : other.core) {
			core[x1] += k1;
			if (core[x1] == 0 && !x1.empty())
				core.erase(x1);
		}
		return *this;
	}
	
	friend IntExpr operator+(IntExpr const& a, IntExpr const& b) {
		IntExpr r(a);
		return r += b;
	}
	
	IntExpr& operator-=(IntExpr const& other) {
		return *this += -other;
	}
	
	friend IntExpr operator-(IntExpr const& a, IntExpr const& b) {
		return a + -b;
	}
	
	IntExpr& operator*=(IntExpr const& other) {
		return *this = *this * other;
	}	
	
	friend IntExpr operator*(IntExpr const& a, IntExpr const& b) {
		if (!a || !b) return {};
		IntExpr r;
		for (auto const& [x1, k1] : a.core) {
			for (auto const& [x2, k2] : b.core) {
				auto k = k1 * k2;
				if (k)
					r += {x1 * x2, k};
			}
		}
		return r;
	}

	friend std::string toTex(IntExpr const& t) {
		if (!t) return "0";
		std::string r;
		CInt C = 0;
		for (auto const& [x1, k1] : t.core) {
			if (x1.empty()) {
				C = k1;
				continue;
			}
			if (!r.empty() && k1 >= 0) 
				r += "+";
			
			if (k1 != 1) {
				if (k1 == -1)
					r += "-";
				else 
					r += toTex(k1);
			}
			r += toTex(x1);	
		}
		if (C != 0) {
			if (!r.empty() && C >= 0) 
				r += "+";
			r += toTex(C);
		}
		return r;
	}

	IntExpr eval(std::string const& name, CInt const& value) const {
		return eval(name, IntExpr{value});
	}
	
	IntExpr eval(std::string const& name, IntExpr const& expr) const {
		core_t ret = core, buf;
		for (auto it = ret.begin(); it != ret.end(); ) {
			if (it->first.contains(name)) {
				buf.insert(*it);
				it = ret.erase(it);
			} else {
				++it;
			}
		}
		IntExpr ans{ret};
		for (auto [x0, k] : buf) {
			mono x = x0;
			CInt n = x.at(name);
			x.erase(name);
			ans += IntExpr(x, k) * pow(expr, n);
		}
		return ans;
	}
	
	
	RatioExpr<CInt> eval(std::string const& name, RatioExpr<CInt> const& expr) const {
		core_t ret = core, buf;
		for (auto it = ret.begin(); it != ret.end();) {
			if (it->first.contains(name)) {
				buf.insert(*it);
				it = ret.erase(it);
			} else {
				++it;
			}
		}
		RatioExpr<CInt> ans{IntExpr<CInt>{ret}, 1};
		for (auto [x0, k] : buf) {
			mono x = x0;
			CInt n = x.at(name);
			x.erase(name);
			ans += IntExpr(x, k) * pow(expr, n);
		}
		return ans;
	}
	
	CInt gcd() const {
		CInt ret = 0;
		for (auto const& [x1, k1] : core) {
			ret = yao_math::gcd(k1, ret);
		}
		return ret;
	}
};

template<typename CInt>
class RatioExpr {
	IntExpr<CInt> num, den;
	void normalize() {
		CInt g = gcd(num.gcd(), den.gcd());
		num.div(g); den.div(g);
	}
public:
	using int_t = CInt;
	RatioExpr(CInt num = 0, CInt den = 1): num{num}, den{den} {}
	RatioExpr(IntExpr<CInt> num, IntExpr<CInt> den = 1)
		: num{num}, den{den} { normalize(); }

	explicit operator bool() const {
		return (bool)num;
	}

	bool operator!() const { return !this->operator bool(); }
	
	RatioExpr operator+() const {
		return *this;
	}
	
	RatioExpr operator-() const {
		return operator*(operator+(), {-1, 1});
	}
	
	friend RatioExpr operator+(RatioExpr const& a, RatioExpr const& b) {
		return {a.num * b.den + b.num * a.den, a.den * b.den};
	}
	
	RatioExpr& operator+=(RatioExpr const& other) {
		return *this = *this + other;
	}	
	
	RatioExpr& operator-=(RatioExpr const& other) {
		return *this += -other;
	}
	
	friend RatioExpr operator-(RatioExpr const& a, RatioExpr const& b) {
		return a +- b;
	}
	
	RatioExpr& operator*=(RatioExpr const& other) {
		return *this = *this * other;
	}
	
	friend RatioExpr operator*(RatioExpr const& a, RatioExpr const& b) {
		if (!a || !b) return {};
		return {a.num * b.num, a.den * b.den};
	}
	
	RatioExpr inverse() const {
		return {den, num};
	}
	
	RatioExpr& operator/=(RatioExpr const& other) {
		return *this *= other.inverse();
	}
	
	friend RatioExpr operator/(RatioExpr const& a, RatioExpr const& b) {
		return a * b.inverse();
	}
	
	RatioExpr eval(std::string const& name, RatioExpr const& value) const {
		return num.eval(name, value) / den.eval(name, value);
	}
	
	friend std::string toTex(RatioExpr const& t) {
		return "\\frac{" + toTex(t.num) + "}{" + toTex(t.den) + "}";
	}
	
};

template<typename Int>
RatioExpr<Int> operator/(IntExpr<Int> const& a, IntExpr<Int> const& b) {
	return {a, b};
}

}

#endif