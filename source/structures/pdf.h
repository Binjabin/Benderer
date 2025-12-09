//
// Created by binjabin on 8/6/25.
//

#ifndef PDF_H
#define PDF_H
#include "../benderer.h"
#include "../structures/onb.h"

class pdf {
public:
    virtual ~pdf() {
    }

    virtual double value(const vec3 &direction) const = 0;

    virtual vec3 generate() const = 0;

    //Do we actually have a value we can sample, or is it a trivial pdf?
    //Example, if we have no in-scene lights
    virtual bool trivial() const = 0;
};

class sphere_pdf : public pdf {
public:
    sphere_pdf() {
    }

    double value(const vec3 &direction) const override {
        return 1 / (4 * pi);
    }

    vec3 generate() const override {
        return random_unit_vector();
    }


    bool trivial() const override { return false; }
};

class cosine_pdf : public pdf {
public:
    cosine_pdf(const vec3 &w) : uvw(w) {
    }

    double value(const vec3 &direction) const override {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta / pi);
    }

    vec3 generate() const override {
        return uvw.transform(random_cosine_direction());
    }

    bool trivial() const override { return false; }

private:
    onb uvw;
};

class hittable_pdf : public pdf {
public:
    hittable_pdf(const hittable &objects, const point3 &origin)
        : objects(objects), origin(origin) {
    }

    double value(const vec3 &direction) const override {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override {
        return objects.random(origin);
    }

    bool trivial() const override { return (objects.get_count() <= 0); }

private:
    const hittable &objects;
    point3 origin;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const vec3 &direction) const override {
        if (p[0]->trivial() && p[1]->trivial()) {
            throw std::runtime_error("Tried to generate from trivial distribution!");
        }
        if (p[0]->trivial()) {
            return p[1]->value(direction);
        }
        if (p[1]->trivial()) {
            return p[0]->value(direction);
        }
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    vec3 generate() const override {
        if (p[0]->trivial() && p[1]->trivial()) {
            throw std::runtime_error("Tried to generate from trivial distribution!");
        }
        if (p[0]->trivial()) {
            return p[1]->generate();
        }
        if (p[1]->trivial()) {
            return p[0]->generate();
        }
        if (random_double() < 0.5) {
            return p[0]->generate();
        }
        return p[1]->generate();
    }

    bool trivial() const override { return (p[0]->trivial() && p[1]->trivial()); }

private:
    shared_ptr<pdf> p[2];
};

#endif //PDF_H
