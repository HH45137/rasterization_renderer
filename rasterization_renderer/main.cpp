#include <iostream>
#include <SDL.h>
#include <glm\glm.hpp>
#include <math.h>
#include <random>
#include <array>
#include <vector>


#define WIN_WIDTH 640 / 2
#define WIN_HEIGHT 480 / 2
#define PIXEL_NUM WIN_WIDTH * WIN_HEIGHT

SDL_Renderer* renderer = nullptr;


typedef glm::vec3 color_t;

struct point2_s
{
	glm::vec2 pos{};
	color_t color{};

	point2_s() :pos(), color() {}
	point2_s(glm::vec2 _pos, color_t _color) :pos(_pos), color(_color) {}
};

// 生成[min, max]范围内的随机整数
int random_int(int min, int max) {
	static std::random_device rd;  // 用于获得随机种子
	static std::mt19937 gen(rd()); // 梅森旋转算法的随机数引擎
	std::uniform_int_distribution<> dis(min, max);
	return dis(gen);
}

float random_float(float min, float max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(min, max);
	return dis(gen);
}

point2_s random_point() {
	return point2_s{
		{ random_int(0,WIN_WIDTH), random_int(0,WIN_HEIGHT)	},
		color_t{random_float(0.0,1.0),random_float(0.0,1.0),random_float(0.0,1.0)}
	};
}

glm::vec2 random_velocity(float minSpeed, float maxSpeed) {
	float angle = random_int(0, 360) * 3.14159f / 180.0f;
	float speed = minSpeed + (maxSpeed - minSpeed) * (random_int(0, 1000) / 1000.0f);
	return glm::vec2{ cos(angle) * speed, sin(angle) * speed };
}

void draw_pixel(point2_s p) {
	p.color *= color_t{ 255.0 };
	SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 255);
	SDL_RenderDrawPoint(renderer, p.pos.x, p.pos.y);
}

struct triangle_s {
private:
	glm::vec3 weights{};

public:
	std::array<point2_s, 3> p{};
	glm::vec2 vel = random_velocity(6.0, 10.0);

	triangle_s() :p(), weights() {};
	triangle_s(point2_s _p0, point2_s _p1, point2_s _p2) :p({ _p0,_p1,_p2 }), weights() {};

	// 判断_p2在_p0和_p1组成的线的左边还是右边，线的方向也会决定左右的定义
	float edge_func(point2_s _p0, point2_s _p1, point2_s _p2) {
		auto tmp1 = (_p1.pos.x - _p0.pos.x) * (_p2.pos.y - _p0.pos.y);
		auto tmp2 = (_p1.pos.y - _p0.pos.y) * (_p2.pos.x - _p0.pos.x);
		return tmp1 - tmp2;
	}

	float get_signed_area() {
		return edge_func(p[0], p[1], p[2]) / 2.0;
	}

	bool is_point_inside(point2_s _p) {
		auto ABP = edge_func(p[0], p[1], _p);
		auto BCP = edge_func(p[1], p[2], _p);
		auto CAP = edge_func(p[2], p[0], _p);
		auto ABC = edge_func(p[0], p[1], p[2]);

		// 归一化权重
		weights[0] = BCP / ABC;
		weights[1] = CAP / ABC;
		weights[2] = ABP / ABC;

		if (ABP >= 0 && BCP >= 0 && CAP >= 0)
		{
			return true;
		}

		return false;
	}

	// 检测数据是否标准，不符合的话返回false
	bool is_true_data() {
		auto ABC = edge_func(p[0], p[1], p[2]);
		auto BCA = edge_func(p[1], p[2], p[0]);
		auto CAB = edge_func(p[2], p[0], p[1]);
		if (ABC < 0 && BCA > 0 && CAB < 0) { return false; }

		return true;
	}

	void draw() {
		auto minX = std::min({ p[0].pos.x, p[1].pos.x, p[2].pos.x });
		auto minY = std::min({ p[0].pos.y, p[1].pos.y, p[2].pos.y });
		auto maxX = std::max({ p[0].pos.x, p[1].pos.x, p[2].pos.x });
		auto maxY = std::max({ p[0].pos.y, p[1].pos.y, p[2].pos.y });

		for (size_t i = minY; i < maxY; i++)
		{
			for (size_t j = minX; j < maxX; j++)
			{
				color_t tmpColor = p[0].color * weights[0] + p[1].color * weights[1] + p[2].color * weights[2];
				point2_s temP{ glm::vec2{j,i}, tmpColor };

				if (is_point_inside(temP) == false) {
					continue;
				}

				draw_pixel(temP);
			}
		}

		return;
	}
};

int main(int argc, char* argv[]) {
	// 生成随机位置随机顶点颜色的三角形
	std::vector<triangle_s> tris;
	{
		for (size_t i = 0; i < 100; i++)
		{
			if (tris.size() >= 10) { break; }

			auto tmpT = triangle_s{
					random_point(),
					random_point(),
					random_point()
			};

			if (tmpT.is_true_data() == false) {
				continue;
			}

			tris.push_back(tmpT);
		}
	}

	SDL_Init(SDL_INIT_VIDEO);

	// 创建窗口和渲染器
	SDL_Window* window = SDL_CreateWindow("software renderer",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT,
		SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_Event event;
	int quit = 0;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) quit = 1;
		}

		// 清除屏幕（黑色）
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		//// 绘制
		//auto tri0 = triangle_s{
		//	{ glm::vec2{125,375}, color_t{1.0,0.0,0.0} },
		//	{ glm::vec2{250,125}, color_t{0.0,1.0,0.0} },
		//	{ glm::vec2{375,375}, color_t{0.0,0.0,1.0} }
		//};
		//tri0.draw();

		for (auto& tri : tris) {
			for (size_t i = 0; i < 3; i++) {
				if (tri.p[i].pos.x < 0 || tri.p[i].pos.x >= WIN_WIDTH) { tri.vel.x = -tri.vel.x; }
				if (tri.p[i].pos.y < 0 || tri.p[i].pos.y >= WIN_HEIGHT) { tri.vel.y = -tri.vel.y; }

				tri.p[i].pos.x = std::max(0.0f, std::min((float)WIN_WIDTH - 1, tri.p[i].pos.x));
				tri.p[i].pos.y = std::max(0.0f, std::min((float)WIN_HEIGHT - 1, tri.p[i].pos.y));

				tri.p[i].pos += tri.vel;
			}
			tri.draw();
		}

		// 更新屏幕
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}

	// 清理资源
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
