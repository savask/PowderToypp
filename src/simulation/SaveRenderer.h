/*
 * SaveRenderer.h
 *
 *  Created on: Apr 3, 2012
 *      Author: Simon
 */

#ifndef SAVERENDERER_H_
#define SAVERENDERER_H_

#include "Singleton.h"
#include "search/Thumbnail.h"
#include "client/GameSave.h"

class Graphics;
class Simulation;
class Renderer;
class SaveRenderer: public Singleton<SaveRenderer> {
	Graphics * g;
	Simulation * sim;
	Renderer * ren;
public:
	SaveRenderer();
	Thumbnail * Render(GameSave * save);
	Thumbnail * Render(unsigned char * saveData, int saveDataSize);
	virtual ~SaveRenderer();

private:
#if defined(OGLR) || defined(OGLI)
	GLuint fboTex, fbo;
#endif
};

#endif /* SAVERENDERER_H_ */
