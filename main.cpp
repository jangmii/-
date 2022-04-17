#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <cstdio>
#include <string>
#include <fstream>
using namespace std;

//config ������
string inputFile;
unsigned int startFrame, numberOfFrames, framesPerSecond;
unsigned int pictureHeight, pictureWidth;
unsigned int frameSize;
void readConfig(int argc, char* args[]);

int main(int argc, char* args[])
{
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	Uint8* tmp = nullptr;
	FILE* fp1 = nullptr;
	Uint32 start, end, time_sum = 0;
	readConfig(argc, args); //config ���� �о����

	const char* filename = inputFile.c_str();
	frameSize = pictureWidth * pictureHeight;
	tmp = new Uint8[frameSize * 3ll / 2];

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return 1;
	}
	if (SDL_CreateWindowAndRenderer(pictureWidth, pictureHeight, SDL_RENDERER_ACCELERATED, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
		return 1;
	}

	fp1 = fopen(filename, "rb"); //����� ���� �б�
	for (auto _ = 0; _ < startFrame; _++) { //���������ӱ����� �а� ����
		fread(tmp, sizeof(Uint8) * frameSize * 3 / 2, 1, fp1);
	}
	for (auto _ = startFrame; _ < numberOfFrames; _++) {
		char title[100];
		sprintf(title, "%s - frame: %d/%d", filename, _ + 1, numberOfFrames); //������ ���� ����
		SDL_SetWindowTitle(window, title);

		start = SDL_GetTicks(); //���۽ð� ����

		fread(tmp, sizeof(Uint8) * frameSize * 3 / 2, 1, fp1);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pictureWidth, pictureHeight);
		if (texture == NULL) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
			return 1;
		}

		SDL_UpdateTexture(texture, NULL, tmp, pictureWidth); //�ؽ��� ������Ʈ
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer); //�ٽ� ������

		end = SDL_GetTicks(); //����ð� ����
		printf("%d�� ������ �ҿ�ð�: %d(ms)\n", _, end - start);
		time_sum += end - start;

		bool running = true, pause = false;
		while (running) { //Ű���� �̺�Ʈ �޾ƿ���
			SDL_Event event; 
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN) {
					switch (event.key.keysym.sym) {
					case SDLK_LEFT: //�Ǿ�����
						fseek(fp1, 0, SEEK_SET);
						_ = startFrame;
						printf("BEGIN\n");
						break;
					case SDLK_RIGHT: //�ǵڷ�
						fseek(fp1, 0, SEEK_END);
						_ = numberOfFrames - 1;
						printf("END\n");
						break;
					case SDLK_SPACE: //�Ͻ�����
						if (pause) printf("RESTART\n");
						else printf("PAUSE\n");
						pause = !pause;
						break;
					}
				}
			}
			if (pause || SDL_GetTicks() - start < 1000.0 / framesPerSecond) { //frame�� ���� �ð���ŭ ��ٸ���
				SDL_Delay(1);
			}
			else {
				running = false;
			}
		}
	}
	delete[] tmp;
	fclose(fp1);
	printf("�� �ҿ�ð�: %d(ms)\n", time_sum);
	
	SDL_Delay(5000);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

void readConfig(int argc, char* args[]) { // https://www.walletfox.com/course/parseconfigfile.php �����Ͽ���
	string line;
	ifstream cFile(argc < 3 ? "config.cfg" : args[2], ios::binary);
	if (!cFile.is_open()) { printf("config file open error.\n"); return; }

	while (getline(cFile, line)) {
		line.erase(remove_if(line.begin(), line.end(), isspace), line.end()); // ���� ����
		if (line[0] == '#' || line.empty()) continue; //�ּ�����

		auto delimiterPos = line.find("=");
		auto name = line.substr(0, delimiterPos);
		auto value = line.substr(delimiterPos + 1);

		if (name == "inputFile") inputFile = value;
		else if (name == "startFrame") startFrame = stoi(value);
		else if (name == "numberOfFrames") numberOfFrames = stoi(value);
		else if (name == "framesPerSecond") framesPerSecond = stoi(value);
		else if (name == "pictureWidth") pictureWidth = stoi(value);
		else if (name == "pictureHeight") pictureHeight = stoi(value);
		printf("%s : %s\n", name.c_str(), value.c_str());
	}
}