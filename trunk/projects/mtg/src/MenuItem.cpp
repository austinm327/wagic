#include "../include/debug.h"
#include "../include/MenuItem.h"


MenuItem::MenuItem(int id, JLBFont *font, const char* text, int x, int y, JQuad * _off, JQuad * _on, const char * particle, JQuad * particleTex, bool hasFocus): JGuiObject(id), mFont(font), mText(text), mX(x), mY(y)
{



	updatedSinceLastRender = 1;
	mParticleSys = NEW hgeParticleSystem(particle, particleTex);
	mParticleSys->MoveTo(mX, mY);


	mHasFocus = hasFocus;
	lastDt = 0.001f;
	mScale = 1.0f;
	mTargetScale = 1.0f;

	onQuad = _on;
	offQuad = _off;

	if (hasFocus)
		Entering();
	mFont->SetScale(1.2f);
	
}

	
void MenuItem::Render()
{

	JRenderer * renderer = JRenderer::GetInstance();
	

	if (mHasFocus)
	{
		if (!updatedSinceLastRender){
			mParticleSys->Update(lastDt);
		}
	// set additive blending
	renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
	mParticleSys->Render();
	// set normal blending
	renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mFont->SetColor(ARGB(255,255,255,255));
		onQuad->SetColor(ARGB(70,255,255,255));
		renderer->RenderQuad(onQuad, SCREEN_WIDTH  , SCREEN_HEIGHT/2 , 0,8,8);
		onQuad->SetColor(ARGB(255,255,255,255));
		mFont->DrawString(mText, SCREEN_WIDTH/2, 3*SCREEN_HEIGHT/4, JGETEXT_CENTER);
		renderer->RenderQuad(onQuad, mX  , mY , 0,mScale,mScale);

	}
	else
	{
		renderer->RenderQuad(offQuad, mX  , mY , 0,mScale,mScale);
	}
	updatedSinceLastRender= 0;
}

void MenuItem::Update(float dt)
{
	updatedSinceLastRender = 1;
	lastDt = dt;
	if (mScale < mTargetScale)
	{
		mScale += 8.0f*dt;
		if (mScale > mTargetScale)
			mScale = mTargetScale;
	}
	else if (mScale > mTargetScale)
	{
		mScale -= 8.0f*dt;
		if (mScale < mTargetScale)
			mScale = mTargetScale;
	}

  if (mHasFocus){
		mParticleSys->Update(dt);
	}

}




void MenuItem::Entering()
{

	mParticleSys->Fire();
	mHasFocus = true;
	mTargetScale = 1.3f;
}


bool MenuItem::Leaving(u32 key)
{
	mParticleSys->Stop(true);
	mHasFocus = false;
	mTargetScale = 1.0f;
	return true;
}


bool MenuItem::ButtonPressed()
{
	return true;
}


MenuItem::~MenuItem(){
	if (mParticleSys) delete mParticleSys;
}
