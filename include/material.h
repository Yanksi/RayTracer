#ifndef MATERIAL_H
#define MATERIAL_H

#include "utils.h"

class material {
public:
	virtual bool scatter(
		const ray& ray_in,
		const hit_record& rec,
		color& attenuation,
		ray& scattered) const = 0;
};

class lambertian : public material {
public:
	color albedo;
public:
	lambertian(const color& a) : albedo(a) {}
	virtual bool scatter(
		const ray& ray_in,
		const hit_record& rec,
		color& attenuation,
		ray& scattered) const override {
		vec3 scatter_direction = rec.normal + random_unit_vector();
		if (near_zero(scatter_direction))
			scatter_direction = rec.normal;
		scattered = ray(rec.p, scatter_direction, ray_in.time);
		attenuation = albedo;
		return true;
	}
};

class metal : public material {
public:
	color albedo;
	double fuzz;
public:
	metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1){}

	virtual bool scatter(
		const ray& ray_in,
		const hit_record& rec,
		color& attenuation,
		ray& scattered) const override {
		vec3 reflected = reflect(ray_in.direction, rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), ray_in.time);
		attenuation = albedo;
		return (scattered.direction.dot(rec.normal) > 0);
	}
};

class dielectric : public material {
public:
	double ir;
private:
	static double reflectance(double cosine, double ref_idx) {
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 *= r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
public:
	dielectric(double index_of_refraction) : ir(index_of_refraction) {}
	virtual bool scatter(
		const ray& r_in,
		const hit_record& rec,
		color& attenuation,
		ray& scattered
	) const override {
		attenuation = color(1.0, 1.0, 1.0);
		double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

		vec3 unit_direction = r_in.direction.normalized();
		double cos_theta = fmin(-rec.normal.dot(unit_direction), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0;
		vec3 direction;

		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
			direction = reflect(unit_direction, rec.normal);
		else
			direction = refract(unit_direction, rec.normal, refraction_ratio);
		 
		scattered = ray(rec.p, direction, r_in.time);
		return true;
	}
};

#endif