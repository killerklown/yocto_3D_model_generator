#include "scene.h"

#include "ext/yocto_math.h"
#include "ext/yocto_utils.h"

shape* facet_normals(shape* shp) {
	auto face = shp;

	auto new_pos = std::vector<vec3f>{};
	auto norm_new = std::vector<vec3f>{};

	auto a = 0;

	for (auto& i : shp->triangles) {
		new_pos.push_back(shp->pos[i.x]);
		i.x = a++;

		new_pos.push_back(shp->pos[i.y]);
		i.y = a++;
		new_pos.push_back(shp->pos[i.z]);
		i.z = a++;

		auto x = triangle_normal(new_pos[i.x], new_pos[i.y], new_pos[i.z]);

		norm_new.push_back(x);
		norm_new.push_back(x);
		norm_new.push_back(x);
	}

	
	shp->norm = norm_new;
	shp->pos = new_pos;

	return shp;
}

shape* displace(shape* shp, texture* disp_txt, float scale) {
	for (auto i = 0; i < shp->pos.size(); i++) {
		shp->pos[i].x += scale * shp->norm[i].x * eval_texture(disp_txt, shp->texcoord[i], false).x;
		shp->pos[i].y += scale * shp->norm[i].y * eval_texture(disp_txt, shp->texcoord[i], false).y;
		shp->pos[i].z += scale * shp->norm[i].z * eval_texture(disp_txt, shp->texcoord[i], false).z;
	}
	compute_smooth_normals(shp);
	return shp;
}

shape* catmull_clark(shape* shp, int level) {
	for (auto i = 0; i < level; i++) {
		ym::tesselate_catmullclark((std::vector<ym::vec4i>&) shp->quads, (std::vector<ym::vec3f>&) shp->pos, (std::vector<ym::vec3f>&) shp->norm, (std::vector<ym::vec2f>&)shp->texcoord, (std::vector<ym::vec4f>&) shp->col, (std::vector<float>&) shp->radius, true);
	}
	return shp;
}

shape* make_quad(const std::string& name, int usteps, int vsteps, float r) {
	auto sh_quad = new shape();
	auto x = (r * 2.0f) / usteps;
	auto y = (r*2.0f) / vsteps;

	for (auto u = 0; u < usteps + 1; u++) {
		for (auto v = 0; v < vsteps + 1; v++) {
			auto ux = (-u * x) / (r * 2.0f);
			auto vy = (-y * y) / (r * 2.0f);

			sh_quad->name = name;

			sh_quad->pos.push_back({ -(u*x) / r + 1.0f, -(v*y) / r + 1.0f , 0.0f });
			sh_quad->norm.push_back({ 0.0f,0.0f,1.0f });
			sh_quad->texcoord.push_back({ (-u*x) / (r * 2.0f) , (-v*y) / (r * 2.0f) });
		}
	}
	for (auto i = 0; i < usteps; i++) {
		for (auto j = 0; j < vsteps; j++) {
			sh_quad->quads.push_back(vec4i{ i *(usteps + 1) + j,(i + 1) * (usteps + 1) + j , (i + 1)*(usteps + 1) + j + 1, i *(usteps + 1) + j + 1 });

		}
	}
	return sh_quad;
}

shape* make_sphere(const std::string& name, int usteps, int vsteps, float r) {
	
	auto sphere = new shape{ name };
	
	auto steps = [usteps](int i, int j) {return j * (usteps + 1) + i; };

	auto u_v_s = (usteps + 1)*(vsteps + 1);
	
	sphere->pos.resize(u_v_s);
	sphere->norm.resize(u_v_s);
	sphere->texcoord.resize(u_v_s);
	
	for (auto j = 0; j < vsteps + 1; j++) {
		for (auto i = 0; i < usteps + 1; i++) {
			auto uv = vec2f{ i / (float)usteps,j / (float)vsteps };
			auto coord_sphe = vec2f{ 2 * pif * uv.x , pif * (1 - uv.y) };
	
			sphere->pos[steps(i, j)] = vec3f{ cos(coord_sphe.x) * sin(coord_sphe.y)*r, sin(coord_sphe.x) * sin(coord_sphe.y)*r, cos(coord_sphe.y)*r };
			sphere->norm[steps(i, j)] = vec3f{ cos(coord_sphe.x) * sin(coord_sphe.y), sin(coord_sphe.x) * sin(coord_sphe.y), cos(coord_sphe.y) };
			sphere->texcoord[steps(i, j)] = uv;
		}
	}
	sphere->quads.resize(usteps*vsteps);
	for (auto j = 0; j < vsteps; j++) {
		for (auto i = 0; i < usteps; i++) {
			sphere->quads[j * usteps + i] = { steps(i,j), steps(i + 1,j), steps(i + 1,j + 1), steps(i,j + 1) };
		}
	}
	return sphere;
}

shape* make_geosphere(const std::string& name, int level, float r) {
    const float X = 0.525731112119133606f;
    const float Z = 0.850650808352039932f;
    auto pos = std::vector<vec3f>{{-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z},
        {X, 0.0, -Z}, {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
        {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}};
    auto triangles = std::vector<vec3i>{{0, 1, 4}, {0, 4, 9}, {9, 4, 5},
        {4, 8, 5}, {4, 1, 8}, {8, 1, 10}, {8, 10, 3}, {5, 8, 3}, {5, 3, 2},
        {2, 3, 7}, {7, 3, 10}, {7, 10, 6}, {7, 6, 11}, {11, 6, 0}, {0, 6, 1},
        {6, 10, 1}, {9, 11, 0}, {9, 2, 11}, {9, 5, 2}, {7, 11, 2}};
    
	auto geo_sphere = new shape{ name };
	geo_sphere->triangles = triangles;
	geo_sphere->pos = pos;
	for (auto i = 0; i < level; i++) {
		tesselate(geo_sphere);
	}
	for (auto& x : geo_sphere->pos) {
		x = normalize(x)*r;
	}
	for (auto& y : geo_sphere->norm) {
		y = normalize(y);
	}
	return geo_sphere;
}

void add_instance(scene* scn, const std::string& name, const frame3f& f,
    shape* shp, material* mat) {
	if (!shp || !mat) return;
	auto ist = new instance();
	ist->name = name;
	ist->shp = shp;
	ist->frame = f;
	ist->mat = mat;
	ist->mat->kd_txt = mat->kd_txt;

	scn->instances.push_back(ist);
	scn->shapes.push_back(shp);
	scn->materials.push_back(mat);
	scn->textures.push_back(mat->kd_txt);
}

material* make_material(const std::string& name, const vec3f& kd, const std::string& kd_txt, const vec3f& ks = { 0.04f, 0.04f, 0.04f }, float rs = 0.01f) {

	auto mat_thi = new material();
	mat_thi->kd = kd;
	mat_thi->name = name;
	mat_thi->kd_txt = new texture{ kd_txt };
	mat_thi->ks = ks;
	mat_thi->rs = rs;

	return mat_thi;
}

void add_sphere_instances(
    scene* scn, const frame3f& f, float R, float r, int num, material* mat) {
    
	auto int_sphere = 2.0f*pif / num;

	auto sphere = make_sphere("sfera", 32, 16, r);

	scn->shapes.push_back(sphere);
	scn->materials.push_back(mat);
	scn->textures.push_back(mat->kd_txt);

	for (int i = 0; i<num; i++) {
		auto ist_sphere = new instance{"sfera"};
		auto x = cos(int_sphere*i)*R;
		auto y = sin(int_sphere*i)*R;
		auto mid_circle = vec3f{ x,y,0.0f };

		ist_sphere->shp = sphere;
		ist_sphere->mat = mat;
		ist_sphere->frame = frame3f{ f.z,f.x,f.y,f.o + mid_circle };
		scn->instances.push_back(ist_sphere);
	}
}

scene* init_scene() {
    auto scn = new scene();
    // add floor
    auto mat = new material{"floor"};
    mat->kd = {0.2f, 0.2f, 0.2f};
    mat->kd_txt = new texture{"grid.png"};
    scn->textures.push_back(mat->kd_txt);
    scn->materials.push_back(mat);
    auto shp = new shape{"floor"};
    shp->pos = {{-20, 0, -20}, {20, 0, -20}, {20, 0, 20}, {-20, 0, 20}};
    shp->norm = {{0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}};
    shp->texcoord = {{-10, -10}, {10, -10}, {10, 10}, {-10, 10}};
    shp->triangles = {{0, 1, 2}, {0, 2, 3}};
    scn->shapes.push_back(shp);
    scn->instances.push_back(new instance{"floor", identity_frame3f, mat, shp});
    // add light
    auto lshp = new shape{"light"};
    lshp->pos = {{1.4f, 8, 6}, {-1.4f, 8, 6}};
    lshp->points = {0, 1};
    scn->shapes.push_back(lshp);
    auto lmat = new material{"light"};
    lmat->ke = {100, 100, 100};
    scn->materials.push_back(lmat);
    scn->instances.push_back(
        new instance{"light", identity_frame3f, lmat, lshp});
    // add camera
    auto cam = new camera{"cam"};
    cam->frame = lookat_frame3f({0, 4, 10}, {0, 1, 0}, {0, 1, 0});
    cam->fovy = 15 * pif / 180.f;
    cam->aspect = 16.0f / 9.0f;
    cam->aperture = 0;
    cam->focus = length(vec3f{0, 4, 10} - vec3f{0, 1, 0});
    scn->cameras.push_back(cam);
    return scn;
}

int main(int argc, char** argv) {
    // command line parsing
    auto parser =
        yu::cmdline::make_parser(argc, argv, "model", "creates simple scenes");
    auto sceneout = yu::cmdline::parse_opts(
        parser, "--output", "-o", "output scene", "out.obj");
    auto type = yu::cmdline::parse_args(
        parser, "type", "type fo scene to create", "empty", true);
    yu::cmdline::check_parser(parser);

    printf("creating scene %s\n", type.c_str());

    // create scene
    auto scn = init_scene();
    if (type == "empty") {
    } else if (type == "simple") {
        add_instance(scn, "quad", make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            make_quad("quad", 16, 16, 1),
            make_material("quad", {1, 1, 1}, "colored.png"));
        add_instance(scn, "sphere", make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            make_sphere("sphere", 32, 16, 1),
            make_material("sphere", {1, 1, 1}, "colored.png"));
    } else if (type == "instances") {
        add_sphere_instances(scn,
            frame3f{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 1.25f, 0}}, 1, 0.1, 16,
            make_material("obj", {1, 1, 1}, "colored.png"));
    } else if (type == "displace") {
        add_instance(scn, "quad1", make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            displace(make_quad("quad1", 64, 64, 1), make_grid_texture(256, 256),
                0.5),
            make_material("quad1", {1, 1, 1}, "colored.png"));
        add_instance(scn, "quad2", make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            displace(make_quad("quad2", 64, 64, 1),
                make_bumpdimple_texture(256, 256), 0.5),
            make_material("quad2", {1, 1, 1}, "colored.png"));
    } else if (type == "normals") {
        add_instance(scn, "smnooth",
            make_frame3_fromz({-1.25f, 1, 0}, {0, 0, 1}),
            make_geosphere("smnooth", 2, 1),
            make_material("smnooth", {0.5f, 0.2f, 0.2f}, ""));
        add_instance(scn, "faceted",
            make_frame3_fromz({1.25f, 1, 0}, {0, 0, 1}),
            facet_normals(make_geosphere("faceted", 2, 1)),
            make_material("faceted", {0.2f, 0.5f, 0.2f}, ""));
    } else if (type == "subdiv") {
        add_instance(scn, "cube",
            make_frame3_fromzx({-1.25f, 1, 0}, {0, 0, 1}, {1, 0, 0}),
            catmull_clark(make_cube("cube"), 4),
            make_material("cube", {0.5f, 0.2f, 0.2f}, ""));
        add_instance(scn, "monkey",
            make_frame3_fromzx({1.25f, 1, 0}, {0, 0, 1}, {1, 0, 0}),
            catmull_clark(make_monkey("monkey"), 2),
            make_material("monkey", {0.2f, 0.5f, 0.2f}, ""));
    } else {
        throw std::runtime_error("bad scene type");
    }

    // save
    printf("saving scene %s\n", sceneout.c_str());
    save_scene(sceneout, scn);
    delete scn;
    return 0;
}
