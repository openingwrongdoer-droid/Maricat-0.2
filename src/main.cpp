#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Shader.h>

struct Texture {
	unsigned int tex;
	int w;
	int h;
	bool issvg;
};

#include <MariCat.h>

#include <unistd.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void processInput(GLFWwindow* window);

void mouseButtonCallBack(GLFWwindow *window, int button, int action, int mods);
void cursorPosCallback(GLFWwindow *window, double xPos, double yPos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

struct key {
	bool pressed = false;
	bool repeat = false;
	bool released = false;
	bool held = false;
};

std::map<std::string, key> keyMap;

void resetMouse();

Texture newTexture (std::string path);

int winWidth = 480;
int winHeight = 360;
std::string winName = "";

struct {
	double x;
	double y;
	bool lHeld;
	bool rHeld;
	bool lClicked;
	bool lReleased;
	bool rClicked;
	bool rReleased;
} mouseInfo;

GLFWwindow* window;

unsigned int VAO, VBO;

Shader mouseCollisionShader, imageShader, lineShader;

Texture currTex;
mariEng::Costume currCostume;

struct {
	float winScale = 1;
} settings;

int main()
{
	std::ifstream settingsFile("settings.txt");
	std::string settingsLine;
	
	getline(settingsFile, settingsLine);
	settingsLine.erase(0, 11);
	try {
		settings.winScale = stof(settingsLine);
	} catch (const std::exception& e) {}
	
	settingsFile.close();
		
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(winWidth * settings.winScale, winHeight * settings.winScale, winName.c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);

	gladLoadGL();
	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
	glfwSetMouseButtonCallback(window, mouseButtonCallBack);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, key_callback);

	float vertices[] =
	{
		// position
		0,   0,
		1,   0,
		1,   1,

		0,   0,
		1,   1,
		0,   1,
		
		0, 	 0,
		0,   1,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);	
	
	mouseCollisionShader.createShader("../shader/image.vs", "../shader/mouseCollision.fs");
	imageShader.createShader("../shader/image.vs", "../shader/image.fs");
	lineShader.createShader("../shader/line.vs", "../shader/line.fs");
	
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	mariEng::start();
	
	glfwSetWindowTitle(window, mariEng::scratchSB3File.substr(2, mariEng::scratchSB3File.size() - 6).c_str());
	
	mariEng::loadingTextures();
	
	for (int i = 0; i < mariEng::sprites.size(); i++) {
		for (int j = 0; j < mariEng::sprites[i].costumes.size(); j++) {
			mariEng::sprites[i].costumes[j].texture = newTexture(mariEng::sprites[i].costumes[j].md5ext);
		}
	}
	
	mariEng::texturesLoaded();
	mariEng::almostthere();
	
	mariEng::greenFlag();
	
	if (mariEng::greetings) std::cout << "\nProgram terminated by user.";
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	
	glfwDestroyWindow(window);
	
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}
void mouseButtonCallBack(GLFWwindow *window, int button, int action, int mods) {
	if (button) {
		if (action && !mouseInfo.rHeld) mouseInfo.rClicked = true;
		else if (!action && mouseInfo.rHeld) mouseInfo.rReleased = true;
		mouseInfo.rHeld = action;
	}
	else  {
		if (action && !mouseInfo.lHeld) mouseInfo.lClicked = true;
		else if (!action && mouseInfo.lHeld) mouseInfo.lReleased = true;
		mouseInfo.lHeld = action;
	}
};

void resetMouse () {
	mouseInfo.lClicked = false;
	mouseInfo.rClicked = false;
	mouseInfo.lReleased = false;
	mouseInfo.lReleased = false;
}
void cursorPosCallback(GLFWwindow *window, double xPos, double yPos) {
	mouseInfo.x = xPos;
	mouseInfo.y = yPos;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	std::string name;
	switch (key) {
		case 262 :
		name = "right arrow";
		break;
		case 263 :
		name = "left arrow";
		break;
		case 264 :
		name = "down arrow";
		break;
		case 265 :
		name = "up arrow";
		break;
		case 32 :
		name = "space";
		break;
		default : name = (char)tolower(key);
	}
	switch (action) {
		case 0 :
		keyMap[name].released = true;
		keyMap[name].repeat = false;
		keyMap[name].held = false;
		keyMap["any"].released = true;
		keyMap["any"].repeat = false;
		keyMap["any"].held = false;
		break;
		case 1 :
		keyMap[name].pressed = true;
		keyMap[name].held = true;
		keyMap["any"].pressed = true;
		keyMap["any"].held = true;
		break;
		case 2:
		keyMap[name].repeat = true;
		keyMap["any"].repeat = true;
		break;
	}
}
void resetKeys () {
	for (auto it = keyMap.begin(); it != keyMap.end(); it++) {
		keyMap[it->first].pressed = false;
		keyMap[it->first].released = false;
	}
}

Texture newTexture (std::string path) {
	/*int err = 0;
	zip_t* za = zip_open(mariEng::scratchSB3File.c_str(), 0, &err);

	if (!za) {
		std::cout << "Fatal error - Failed to open .sb3 file.";
	}

	zip_stat_t st;
	if (zip_stat(za, path.c_str(), 0, &st) != 0) {
		zip_close(za);
		std::cout << "Fatal error - " << path << " not found in archive.";
	}

	zip_file_t* zf = zip_fopen(za, path.c_str(), 0);
	if (!zf) {
		zip_close(za);
		std::cout << "Fatal error - Failed to open " << path << ".";
	}

	std::string content;
	content.resize(st.size);

	zip_int64_t bytes_read = zip_fread(zf, content.data(), st.size);
	zip_fclose(zf);
	zip_close(za);

	if (bytes_read < 0) {
		std::cout << "Fatal error - Failed to read " << path << ".";
	}*/
	
	std::string command = "zipExtractor.exe \"" + mariEng::scratchSB3File + "\" " + path;
	system(command.c_str());
	
	Texture t;
	
	unsigned int texture1;

	glGenTextures(1, &texture1);

	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height;
	
	if (path.substr(path.size() - 3, 3) == "svg") {
		t.issvg = true;
		
		command = "resvg.exe --quiet " + path + " temp.png";
		std::system(command.c_str());
		
		int nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load("temp.png", &width, &height, &nrChannels, 0);
		
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
		std::remove("temp.png");
	} else {
		t.issvg = false;
	
		int nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}
	std::remove(path.c_str());
	
	t.tex = texture1;
	t.w = width;
	t.h = height;

	return t;
}
bool frameAdvance () {
	if (glfwWindowShouldClose(window)) return 1;
	resetMouse();
	resetKeys();
	processInput(window);
	glClearDepth(0.0);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	imageShader.use();
	glBindVertexArray(VAO);
	for (int i = 0; i < mariEng::sprites.size(); i++) {
		if (!mariEng::sprites[i].visible) continue;
		currCostume = mariEng::sprites[i].costumes[mariEng::sprites[i].currentCostume];
		currTex = currCostume.texture;
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currTex.tex);
		imageShader.setVec2("size", currTex.w, currTex.h);
		imageShader.setFloat("scale", (float)mariEng::sprites[i].size);
		imageShader.setVec2("pos", mariEng::sprites[i].x, mariEng::sprites[i].y);
		imageShader.setVec2("costCenter", currCostume.rotationCenterX, currCostume.rotationCenterY);
		imageShader.setInt("issvg", currTex.issvg);
		imageShader.setFloat("layer", (float)mariEng::sprites[i].layerOrder / mariEng::sprites.size());
		imageShader.setFloat("dir", (mariEng::sprites[i].direction + 90) / 180 * 3.1415f - 3.1415);
		std::string rotStr = mariEng::sprites[i].rotationStyle;
		int rotStyle = rotStr[0] == 'l' ? 0 : rotStr[0] == 'd' ? 1 : 2;
		imageShader.setInt("rotStyle", rotStyle);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glfwSwapBuffers(window);
	glfwPollEvents();
	usleep(999);
	mariEng::mouse.x = mouseInfo.x / settings.winScale - 240;
	mariEng::mouse.y = mouseInfo.y / settings.winScale - 180;
	mariEng::mouse.action = mouseInfo.lClicked | (mouseInfo.lHeld << 1);
	mariEng::globalSens.timer = double(clock() - mariEng::programStart) / CLOCKS_PER_SEC;
	return 0;
}

bool mouseCollisionCheck (int x, int y, int i) {
	glClearDepth(0.0);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mouseCollisionShader.use();
	glBindVertexArray(VAO);
	currCostume = mariEng::sprites[i].costumes[mariEng::sprites[i].currentCostume];
	currTex = currCostume.texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currTex.tex);
	mouseCollisionShader.setVec2("size", currTex.w, currTex.h);
	mouseCollisionShader.setFloat("scale", (float)mariEng::sprites[i].size);
	mouseCollisionShader.setVec2("pos", mariEng::sprites[i].x, mariEng::sprites[i].y);
	mouseCollisionShader.setVec2("costCenter", currCostume.rotationCenterX, currCostume.rotationCenterY);
	mouseCollisionShader.setInt("issvg", currTex.issvg);
	mouseCollisionShader.setFloat("layer", (float)mariEng::sprites[i].layerOrder / mariEng::sprites.size());
	mouseCollisionShader.setFloat("dir", (mariEng::sprites[i].direction + 90) / 180 * 3.1415f - 3.1415);
	std::string rotStr = mariEng::sprites[i].rotationStyle;
	int rotStyle = rotStr[0] == 'l' ? 0 : rotStr[0] == 'd' ? 1 : 2;
	mouseCollisionShader.setInt("rotStyle", rotStyle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	GLubyte pixel;
	glReadPixels(x + 240, -y + 180, 1, 1, GL_RED, GL_UNSIGNED_BYTE, &pixel);
	return pixel == 128;
}

bool spriteCollisionCheck (int i, int j) {
	if (mariEng::sprites[i].visible && mariEng::sprites[j].visible != 1) return 0;
	glClearDepth(0.0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mouseCollisionShader.use();
	glBindVertexArray(VAO);
	currCostume = mariEng::sprites[i].costumes[mariEng::sprites[i].currentCostume];
	currTex = currCostume.texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currTex.tex);
	mouseCollisionShader.setVec2("size", currTex.w, currTex.h);
	mouseCollisionShader.setFloat("scale", (float)mariEng::sprites[i].size);
	mouseCollisionShader.setVec2("pos", mariEng::sprites[i].x, mariEng::sprites[i].y);
	mouseCollisionShader.setVec2("costCenter", currCostume.rotationCenterX, currCostume.rotationCenterY);
	mouseCollisionShader.setInt("issvg", currTex.issvg);
	mouseCollisionShader.setFloat("layer", (float)mariEng::sprites[i].layerOrder / mariEng::sprites.size());
	mouseCollisionShader.setFloat("dir", (mariEng::sprites[i].direction + 90) / 180 * 3.1415f - 3.1415);
	std::string rotStr = mariEng::sprites[i].rotationStyle;
	int rotStyle = rotStr[0] == 'l' ? 0 : rotStr[0] == 'd' ? 1 : 2;
	mouseCollisionShader.setInt("rotStyle", rotStyle);
	glDepthMask(GL_FALSE);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	i = j;
	
	currCostume = mariEng::sprites[i].costumes[mariEng::sprites[i].currentCostume];
	currTex = currCostume.texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currTex.tex);
	mouseCollisionShader.setVec2("size", currTex.w, currTex.h);
	mouseCollisionShader.setFloat("scale", (float)mariEng::sprites[i].size);
	mouseCollisionShader.setVec2("pos", mariEng::sprites[i].x, mariEng::sprites[i].y);
	mouseCollisionShader.setVec2("costCenter", currCostume.rotationCenterX, currCostume.rotationCenterY);
	mouseCollisionShader.setInt("issvg", currTex.issvg);
	mouseCollisionShader.setFloat("layer", (float)mariEng::sprites[i].layerOrder / mariEng::sprites.size());
	mouseCollisionShader.setFloat("dir", (mariEng::sprites[i].direction + 90) / 180 * 3.1415f - 3.1415);
	rotStr = mariEng::sprites[i].rotationStyle;
	rotStyle = rotStr[0] == 'l' ? 0 : rotStr[0] == 'd' ? 1 : 2;
	mouseCollisionShader.setInt("rotStyle", rotStyle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDepthMask(GL_TRUE);
	int size = 480 * settings.winScale * 360 * settings.winScale;
	GLubyte *pixel = new GLubyte[size];
	glReadPixels(0, 0, (unsigned int)(480 * settings.winScale), (unsigned int)(360 * settings.winScale), GL_ALPHA, GL_UNSIGNED_BYTE, pixel);
	for (int l = 0; l < size; l++) {
		if (pixel[l] == 96) {
		delete[] pixel;
			return 1;
		}
	}
	
	delete[] pixel;
	
	return 0;
}

bool edgeCollisionCheck (int i) {
	glClearDepth(0.0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mouseCollisionShader.use();
	glBindVertexArray(VAO);
	currCostume = mariEng::sprites[i].costumes[mariEng::sprites[i].currentCostume];
	currTex = currCostume.texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currTex.tex);
	mouseCollisionShader.setVec2("size", currTex.w, currTex.h);
	mouseCollisionShader.setFloat("scale", (float)mariEng::sprites[i].size);
	mouseCollisionShader.setVec2("pos", mariEng::sprites[i].x, mariEng::sprites[i].y);
	mouseCollisionShader.setVec2("costCenter", currCostume.rotationCenterX, currCostume.rotationCenterY);
	mouseCollisionShader.setInt("issvg", currTex.issvg);
	mouseCollisionShader.setFloat("layer", (float)mariEng::sprites[i].layerOrder / mariEng::sprites.size());
	mouseCollisionShader.setFloat("dir", (mariEng::sprites[i].direction + 90) / 180 * 3.1415f - 3.1415);
	std::string rotStr = mariEng::sprites[i].rotationStyle;
	int rotStyle = rotStr[0] == 'l' ? 0 : rotStr[0] == 'd' ? 1 : 2;
	mouseCollisionShader.setInt("rotStyle", rotStyle);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glLineWidth(2.0f);
	lineShader.use();
	//TODO
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 4, 6);
	glDrawArrays(GL_LINES, 1, 3);
	glDrawArrays(GL_LINES, 7, 8);
	
	int size = 480 * settings.winScale * 360 * settings.winScale;
	GLubyte *pixel = new GLubyte[size];
	glReadPixels(0, 0, (unsigned int)(480 * settings.winScale), (unsigned int)(360 * settings.winScale), GL_ALPHA, GL_UNSIGNED_BYTE, pixel);
	for (int l = 0; l < size; l++) {
		if (pixel[l] == 96) {
			return 1;
			delete[] pixel;
			return 1;
		}
	}
	delete[] pixel;
	
	return 0;
}

short getKeyData(std::string name) {
	key thisKey = keyMap[name];
	return thisKey.pressed | (thisKey.repeat << 1) | (thisKey.held << 2);
}