#pragma once

#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Eunoia {

	class EU_API MasterRenderer
	{
	public:
		MasterRenderer(RenderContext* rc, Display* display);
		~MasterRenderer();

		void Init();
		void BeginFrame();
		void EndFrame();
		void RenderFrame();

		Renderer2D* GetRenderer2D();
		Renderer3D* GetRenderer3D();

		TextureID GetFinalOutput();

		void GetOutputSize(u32* width, u32* height);
		void ResizeOutput(u32 width, u32 height);
	private:
		RenderPassID m_RenderPass;
		RenderCommand m_DrawQuad;

		Renderer2D m_Renderer2D;
		Renderer3D m_Renderer3D;
		TextureID m_Output2D;
		TextureID m_Output3D;
		TextureID m_FinalOutput; // If editor is attached, the final output will render to this texture. If this is the EU_DIST build then
								 // the final output will be rendered to the swapchain
	};

}