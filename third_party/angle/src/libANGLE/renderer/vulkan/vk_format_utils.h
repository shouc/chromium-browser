//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_format_utils:
//   Helper for Vulkan format code.

#ifndef LIBANGLE_RENDERER_VULKAN_VK_FORMAT_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_VK_FORMAT_UTILS_H_

#include <vulkan/vulkan.h>

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/copyvertex.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "platform/FeaturesVk.h"

#include <array>

namespace gl
{
struct SwizzleState;
class TextureCapsMap;
}  // namespace gl

namespace rx
{
class RendererVk;
class ContextVk;

namespace vk
{
// VkFormat values in range [0, kNumVkFormats) are used as indices in various tables.
constexpr uint32_t kNumVkFormats = 185;

struct ImageFormatInitInfo final
{
    angle::FormatID format;
    VkFormat vkFormat;
    InitializeTextureDataFunction initializer;
};

struct BufferFormatInitInfo final
{
    angle::FormatID format;
    VkFormat vkFormat;
    bool vkFormatIsPacked;
    VertexCopyFunction vertexLoadFunction;
    bool vertexLoadRequiresConversion;
};

// Describes a Vulkan format. For more information on formats in the Vulkan back-end please see
// https://chromium.googlesource.com/angle/angle/+/master/src/libANGLE/renderer/vulkan/doc/FormatTablesAndEmulation.md
struct Format final : private angle::NonCopyable
{
    Format();

    bool valid() const { return internalFormat != 0; }

    // The intended format is the front-end format. For Textures this usually correponds to a
    // GLenum in the headers. Buffer formats don't always have a corresponding GLenum type.
    // Some Surface formats and unsized types also don't have a corresponding GLenum.
    const angle::Format &intendedFormat() const { return angle::Format::Get(intendedFormatID); }

    // The actual Image format is used to implement the front-end format for Texture/Renderbuffers.
    const angle::Format &actualImageFormat() const
    {
        return angle::Format::Get(actualImageFormatID);
    }

    // The actual Buffer format is used to implement the front-end format for Buffers.
    const angle::Format &actualBufferFormat() const
    {
        return angle::Format::Get(actualBufferFormatID);
    }

    // The |internalFormat| always correponds to a valid GLenum type. For types that don't have a
    // corresponding GLenum we do our best to specify a GLenum that is "close".
    const gl::InternalFormat &getInternalFormatInfo(GLenum type) const
    {
        return gl::GetInternalFormatInfo(internalFormat, type);
    }

    // Returns buffer alignment for image-copy operations (to or from a buffer).
    size_t getImageCopyBufferAlignment() const;

    // Returns true if the Image format has more channels than the ANGLE format.
    bool hasEmulatedImageChannels() const;

    // This is an auto-generated method in vk_format_table_autogen.cpp.
    void initialize(RendererVk *renderer, const angle::Format &angleFormat);

    // These are used in the format table init.
    void initImageFallback(RendererVk *renderer, const ImageFormatInitInfo *info, int numInfo);
    void initBufferFallback(RendererVk *renderer, const BufferFormatInitInfo *info, int numInfo);

    angle::FormatID intendedFormatID;
    GLenum internalFormat;
    angle::FormatID actualImageFormatID;
    VkFormat vkImageFormat;
    angle::FormatID actualBufferFormatID;
    VkFormat vkBufferFormat;

    InitializeTextureDataFunction imageInitializerFunction;
    LoadFunctionMap textureLoadFunctions;
    VertexCopyFunction vertexLoadFunction;

    bool vertexLoadRequiresConversion;
    bool vkBufferFormatIsPacked;
    bool vkFormatIsInt;
    bool vkFormatIsUnsigned;
};

bool operator==(const Format &lhs, const Format &rhs);
bool operator!=(const Format &lhs, const Format &rhs);

class FormatTable final : angle::NonCopyable
{
  public:
    FormatTable();
    ~FormatTable();

    // Also initializes the TextureCapsMap and the compressedTextureCaps in the Caps instance.
    void initialize(RendererVk *renderer,
                    gl::TextureCapsMap *outTextureCapsMap,
                    std::vector<GLenum> *outCompressedTextureFormats);

    ANGLE_INLINE const Format &operator[](GLenum internalFormat) const
    {
        angle::FormatID formatID = angle::Format::InternalFormatToID(internalFormat);
        return mFormatData[static_cast<size_t>(formatID)];
    }

    ANGLE_INLINE const Format &operator[](angle::FormatID formatID) const
    {
        return mFormatData[static_cast<size_t>(formatID)];
    }

  private:
    // The table data is indexed by angle::FormatID.
    std::array<Format, angle::kNumANGLEFormats> mFormatData;
};

// This will return a reference to a VkFormatProperties with the feature flags supported
// if the format is a mandatory format described in section 31.3.3. Required Format Support
// of the Vulkan spec. If the vkFormat isn't mandatory, it will return a VkFormatProperties
// initialized to 0.
const VkFormatProperties &GetMandatoryFormatSupport(VkFormat vkFormat);

VkImageUsageFlags GetMaximalImageUsageFlags(RendererVk *renderer, VkFormat format);

}  // namespace vk

// Checks if a vkFormat supports all the features needed to use it as a GL texture format
bool HasFullTextureFormatSupport(RendererVk *renderer, VkFormat vkFormat);
// Checks if a vkFormat supports all the features except texture filtering
bool HasNonFilterableTextureFormatSupport(RendererVk *renderer, VkFormat vkFormat);
// Checks if a vkFormat supports all the features except rendering
bool HasNonRenderableTextureFormatSupport(RendererVk *renderer, VkFormat vkFormat);

// Returns the alignment for a buffer to be used with the vertex input stage in Vulkan. This
// calculation is listed in the Vulkan spec at the end of the section 'Vertex Input Description'.
size_t GetVertexInputAlignment(const vk::Format &format);

void MapSwizzleState(const ContextVk *contextVk,
                     const vk::Format &format,
                     const bool sized,
                     const gl::SwizzleState &swizzleState,
                     gl::SwizzleState *swizzleStateOut);
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VK_FORMAT_UTILS_H_
