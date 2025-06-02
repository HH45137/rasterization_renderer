#include <iostream>
#include <SDL.h>
#include <glm\glm.hpp>
#include <math.h>
#include <random>


#define WIN_WIDTH 640
#define WIN_HEIGHT 480
#define PIXEL_NUM WIN_WIDTH * WIN_HEIGHT

SDL_Renderer* renderer = nullptr;


typedef glm::vec3 color_t;

struct point2_s
{
	glm::vec2 pos{};
	color_t color{};
};

// 生成[min, max]范围内的随机整数
int randomInt(int min, int max) {
	static std::random_device rd;  // 用于获得随机种子
	static std::mt19937 gen(rd()); // 梅森旋转算法的随机数引擎
	std::uniform_int_distribution<> dis(min, max);
	return dis(gen);
}

void draw_pixel(point2_s p) {
	p.color *= color_t{ 255.0 };
	SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, 255);
	SDL_RenderDrawPoint(renderer, p.pos.x, p.pos.y);
}

struct triangle_s {
	point2_s p0{}, p1{}, p2{};

	float edge_func(point2_s _p0, point2_s _p1, point2_s _p2) {
		auto tmp1 = (_p1.pos.x - _p0.pos.x) * (_p2.pos.y - _p0.pos.y);
		auto tmp2 = (_p1.pos.y - _p0.pos.y) * (_p2.pos.x - _p0.pos.x);
		return tmp1 - tmp2;
	}

	float get_signed_area() {
		return edge_func(p0, p1, p2) / 2.0;
	}

	bool is_point_inside(point2_s p) {
		const auto ABP = edge_func(p0, p1, p);
		const auto BCP = edge_func(p1, p2, p);
		const auto CAP = edge_func(p2, p0, p);

		if (ABP >= 0 && BCP >= 0 && CAP >= 0)
		{
			return true;
		}

		return false;
	}

	void draw() {
		auto minX = std::min({ p0.pos.x, p1.pos.x, p2.pos.x });
		auto minY = std::min({ p0.pos.y, p1.pos.y, p2.pos.y });
		auto maxX = std::max({ p0.pos.x, p1.pos.x, p2.pos.x });
		auto maxY = std::max({ p0.pos.y, p1.pos.y, p2.pos.y });

		for (size_t i = minY; i < maxY; i++)
		{
			for (size_t j = minX; j < maxX; j++)
			{
				point2_s temP{ glm::vec2{j,i},color_t{1,0,0} };

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

		// 绘制
		auto tri0 = triangle_s{
			{ glm::vec2{125,375}, color_t{1.0,0.0,0.0} },
			{ glm::vec2{250,125}, color_t{0.0,1.0,0.0} },
			{ glm::vec2{375,375}, color_t{0.0,0.0,1.0} }
		};
		tri0.draw();

		tri0.p0.pos.x = randomInt(0, WIN_WIDTH);
		tri0.p0.pos.y = randomInt(0, WIN_HEIGHT);
		tri0.p1.pos.x = randomInt(0, WIN_WIDTH);
		tri0.p1.pos.y = randomInt(0, WIN_HEIGHT);
		tri0.p2.pos.x = randomInt(0, WIN_WIDTH);
		tri0.p2.pos.y = randomInt(0, WIN_HEIGHT);
		tri0.draw();

		// 更新屏幕
		SDL_RenderPresent(renderer);
	}

	// 清理资源
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
