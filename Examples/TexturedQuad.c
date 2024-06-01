#include <SDL_gpu_examples.h>

static const char* SamplerNames[] =
{
	"PointClamp",
	"PointWrap",
	"LinearClamp",
	"LinearWrap",
	"AnisotropicClamp",
	"AnisotropicWrap",
};

static SDL_GpuGraphicsPipeline* Pipeline;
static SDL_GpuBuffer* VertexBuffer;
static SDL_GpuBuffer* IndexBuffer;
static SDL_GpuTexture* Texture;
static SDL_GpuSampler* Samplers[SDL_arraysize(SamplerNames)];

static int CurrentSamplerIndex;

static int Init(Context* context)
{
	int result = CommonInit(context, 0);
	if (result < 0)
	{
		return result;
	}

	// Create the shaders
	SDL_GpuShader* vertexShader = LoadShader(context->Device, "TexturedQuad.vert.spv");
	if (vertexShader == NULL)
	{
		SDL_Log("Failed to create vertex shader!");
		return -1;
	}

	SDL_GpuShader* fragmentShader = LoadShader(context->Device, "TexturedQuad.frag.spv");
	if (fragmentShader == NULL)
	{
		SDL_Log("Failed to create fragment shader!");
		return -1;
	}

	// Load the image
	int img_x, img_y, n;
	char *imageData = LoadImage("ravioli.png", &img_x, &img_y, &n, 4, SDL_FALSE);
	if (imageData == NULL)
	{
		SDL_Log("Could not load image data!");
		return -1;
	}

	// Create the pipeline
	SDL_GpuGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.attachmentInfo = {
			.colorAttachmentCount = 1,
			.colorAttachmentDescriptions = (SDL_GpuColorAttachmentDescription[]){{
				.format = SDL_GpuGetSwapchainTextureFormat(context->Device, context->Window),
				.blendState = {
					.blendEnable = SDL_TRUE,
					.alphaBlendOp = SDL_GPU_BLENDOP_ADD,
					.colorBlendOp = SDL_GPU_BLENDOP_ADD,
					.colorWriteMask = 0xF,
					.srcColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
					.srcAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
					.dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ZERO,
					.dstAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ZERO
				}
			}},
		},
		.vertexInputState = (SDL_GpuVertexInputState){
			.vertexBindingCount = 1,
			.vertexBindings = (SDL_GpuVertexBinding[]){{
				.binding = 0,
				.inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.stepRate = 0,
				.stride = sizeof(PositionTextureVertex)
			}},
			.vertexAttributeCount = 2,
			.vertexAttributes = (SDL_GpuVertexAttribute[]){{
				.binding = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_VECTOR3,
				.location = 0,
				.offset = 0
			}, {
				.binding = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_VECTOR2,
				.location = 1,
				.offset = sizeof(float) * 3
			}}
		},
		.multisampleState.sampleMask = 0xFFFF,
		.primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertexShader = vertexShader,
		.fragmentShader = fragmentShader,
		.fragmentResourceLayoutInfo.samplerCount = 1
	};

	Pipeline = SDL_GpuCreateGraphicsPipeline(context->Device, &pipelineCreateInfo);
	if (Pipeline == NULL)
	{
		SDL_Log("Failed to create pipeline!");
		return -1;
	}

	SDL_GpuReleaseShader(context->Device, vertexShader);
	SDL_GpuReleaseShader(context->Device, fragmentShader);

	// PointClamp
	Samplers[0] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_NEAREST,
		.magFilter = SDL_GPU_FILTER_NEAREST,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});
	// PointWrap
	Samplers[1] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_NEAREST,
		.magFilter = SDL_GPU_FILTER_NEAREST,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});
	// LinearClamp
	Samplers[2] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_LINEAR,
		.magFilter = SDL_GPU_FILTER_LINEAR,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});
	// LinearWrap
	Samplers[3] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_LINEAR,
		.magFilter = SDL_GPU_FILTER_LINEAR,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});
	// AnisotropicClamp
	Samplers[4] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_LINEAR,
		.magFilter = SDL_GPU_FILTER_LINEAR,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.anisotropyEnable = SDL_TRUE,
		.maxAnisotropy = 4
	});
	// AnisotropicWrap
	Samplers[5] = SDL_GpuCreateSampler(context->Device, &(SDL_GpuSamplerStateCreateInfo){
		.minFilter = SDL_GPU_FILTER_LINEAR,
		.magFilter = SDL_GPU_FILTER_LINEAR,
		.mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.addressModeU = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeV = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.addressModeW = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.anisotropyEnable = SDL_TRUE,
		.maxAnisotropy = 4
	});

	// Create the GPU resources
	VertexBuffer = SDL_GpuCreateGpuBuffer(
		context->Device,
		SDL_GPU_BUFFERUSAGE_VERTEX_BIT,
		sizeof(PositionTextureVertex) * 4
	);

	IndexBuffer = SDL_GpuCreateGpuBuffer(
		context->Device,
		SDL_GPU_BUFFERUSAGE_INDEX_BIT,
		sizeof(Uint16) * 6
	);

	Texture = SDL_GpuCreateTexture(context->Device, &(SDL_GpuTextureCreateInfo){
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8,
		.width = img_x,
		.height = img_y,
		.depth = 1,
		.layerCount = 1,
		.levelCount = 1,
		.usageFlags = SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT
	});

	// Upload GPU data
	SDL_GpuTransferBuffer* transferBuffer = SDL_GpuCreateTransferBuffer(
		context->Device,
		SDL_GPU_TRANSFERUSAGE_BUFFER,
		SDL_GPU_TRANSFER_MAP_WRITE,
		(sizeof(PositionTextureVertex) * 4) +
			(sizeof(Uint16) * 6) +
			(img_x * img_y * 4)
	);

	PositionTextureVertex* transferData;
	SDL_GpuMapTransferBuffer(
		context->Device,
		transferBuffer,
		SDL_FALSE,
		(void**) &transferData
	);

	transferData[0] = (PositionTextureVertex) { -1,  1, 0, 0, 0 };
	transferData[1] = (PositionTextureVertex) {  1,  1, 0, 4, 0 };
	transferData[2] = (PositionTextureVertex) {  1, -1, 0, 4, 4 };
	transferData[3] = (PositionTextureVertex) { -1, -1, 0, 0, 4 };

	Uint16* indexData = (Uint16*) &transferData[4];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 0;
	indexData[4] = 2;
	indexData[5] = 3;

	SDL_memcpy((void*) &indexData[6], imageData, img_x * img_y * 4);

	SDL_GpuUnmapTransferBuffer(context->Device, transferBuffer);
	SDL_free(imageData);

	// Upload the transfer data to the GPU resources
	SDL_GpuCommandBuffer* uploadCmdBuf = SDL_GpuAcquireCommandBuffer(context->Device);
	SDL_GpuCopyPass* copyPass = SDL_GpuBeginCopyPass(uploadCmdBuf);

	SDL_GpuUploadToBuffer(
		copyPass,
		transferBuffer,
		VertexBuffer,
		&(SDL_GpuBufferCopy){
			.srcOffset = 0,
			.dstOffset = 0,
			.size = sizeof(PositionTextureVertex) * 4
		},
		SDL_FALSE
	);

	SDL_GpuUploadToBuffer(
		copyPass,
		transferBuffer,
		IndexBuffer,
		&(SDL_GpuBufferCopy){
			.srcOffset = sizeof(PositionTextureVertex) * 4,
			.dstOffset = 0,
			.size = sizeof(Uint16) * 6
		},
		SDL_FALSE
	);

	SDL_GpuUploadToTexture(
		copyPass,
		transferBuffer,
		&(SDL_GpuTextureRegion){
			.textureSlice.texture = Texture,
			.w = img_x,
			.h = img_y,
			.d = 1
		},
		&(SDL_GpuBufferImageCopy){
			.bufferOffset =
				(sizeof(PositionTextureVertex) * 4) +
				(sizeof(Uint16) * 6),
		},
		SDL_FALSE
	);

	SDL_GpuEndCopyPass(copyPass);
	SDL_GpuSubmit(uploadCmdBuf);
	SDL_GpuReleaseTransferBuffer(context->Device, transferBuffer);

	// Finally, print instructions!
	SDL_Log("Press Left/Right to switch between sampler states");
	SDL_Log("Setting sampler state to: %s", SamplerNames[0]);

	return 0;
}

static int Update(Context* context)
{
	if (context->LeftPressed)
	{
		CurrentSamplerIndex -= 1;
		if (CurrentSamplerIndex < 0)
		{
			CurrentSamplerIndex = SDL_arraysize(Samplers) - 1;
		}
		SDL_Log("Setting sampler state to: %s", SamplerNames[CurrentSamplerIndex]);
	}

	if (context->RightPressed)
	{
		CurrentSamplerIndex = (CurrentSamplerIndex + 1) % SDL_arraysize(Samplers);
		SDL_Log("Setting sampler state to: %s", SamplerNames[CurrentSamplerIndex]);
	}

	return 0;
}

static int Draw(Context* context)
{
	SDL_GpuCommandBuffer* cmdbuf = SDL_GpuAcquireCommandBuffer(context->Device);
	if (cmdbuf == NULL)
	{
		SDL_Log("GpuAcquireCommandBuffer failed");
		return -1;
	}

	Uint32 w, h;
	SDL_GpuTexture* swapchainTexture = SDL_GpuAcquireSwapchainTexture(cmdbuf, context->Window, &w, &h);
	if (swapchainTexture != NULL)
	{
		SDL_GpuColorAttachmentInfo colorAttachmentInfo = { 0 };
		colorAttachmentInfo.textureSlice.texture = swapchainTexture;
		colorAttachmentInfo.clearColor = (SDL_GpuColor){ 0.0f, 0.0f, 0.0f, 1.0f };
		colorAttachmentInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
		colorAttachmentInfo.storeOp = SDL_GPU_STOREOP_STORE;

		SDL_GpuRenderPass* renderPass = SDL_GpuBeginRenderPass(cmdbuf, &colorAttachmentInfo, 1, NULL);

		SDL_GpuBindGraphicsPipeline(renderPass, Pipeline);
		SDL_GpuBindVertexBuffers(renderPass, 0, &(SDL_GpuBufferBinding){ .gpuBuffer = VertexBuffer, .offset = 0 }, 1);
		SDL_GpuBindIndexBuffer(renderPass, &(SDL_GpuBufferBinding){ .gpuBuffer = IndexBuffer, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		SDL_GpuBindFragmentSamplers(renderPass, 0, &(SDL_GpuTextureSamplerBinding){ .texture = Texture, .sampler = Samplers[CurrentSamplerIndex] }, 1);
		SDL_GpuDrawInstancedPrimitives(renderPass, 0, 0, 2, 1);

		SDL_GpuEndRenderPass(renderPass);
	}

	SDL_GpuSubmit(cmdbuf);

	return 0;
}

static void Quit(Context* context)
{
	SDL_GpuReleaseGraphicsPipeline(context->Device, Pipeline);
	SDL_GpuReleaseGpuBuffer(context->Device, VertexBuffer);
	SDL_GpuReleaseGpuBuffer(context->Device, IndexBuffer);
	SDL_GpuReleaseTexture(context->Device, Texture);

	for (int i = 0; i < SDL_arraysize(Samplers); i += 1)
	{
		SDL_GpuReleaseSampler(context->Device, Samplers[i]);
	}

	CurrentSamplerIndex = 0;

	CommonQuit(context);
}

Example TexturedQuad_Example = { "TexturedQuad", Init, Update, Draw, Quit };
