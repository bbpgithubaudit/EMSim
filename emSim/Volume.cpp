/* Copyright (c) 2017-2020, EPFL/Blue Brain Project
 * Responsible Author: Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
 *
 * This file is part of EMSim <https://github.com/BlueBrain/EMSim>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <emSim/Volume.h>

namespace ems
{
Volume::Volume(const glm::vec3& voxelSize, const glm::vec3& extent,
               const EventsAABB& circuitAABB)
    : _voxelSize(voxelSize)
    , _volumeSize(glm::uvec3(
          (circuitAABB.max.x - circuitAABB.min.x + extent.x) / voxelSize.x +
              0.5f,
          (circuitAABB.max.y - circuitAABB.min.y + extent.y) / voxelSize.y +
              0.5f,
          (circuitAABB.max.z - circuitAABB.min.z + extent.z) / voxelSize.z +
              0.5f))
    , _data(alignedMalloc<float>(_getVoxelCount()))
    , _origin(glm::vec3(circuitAABB.min.x - extent.x / 2.0f,
                        circuitAABB.min.y - extent.y / 2.0f,
                        circuitAABB.min.z - extent.z / 2.0f))
{
    std::cout << "INFO: Volume size is [" << _volumeSize.x << " "
              << _volumeSize.y << " " << _volumeSize.z << "]" << std::endl;
    std::memset(_data.get(), 0.0f, _getVoxelCount() * sizeof(float));
}

void Volume::clear(const float value)
{
    std::memset(_data.get(), value, _getVoxelCount() * sizeof(float));
}

void Volume::writeToFile(const float time, const float timeStep, 
                         const std::string& dataUnit, 
                         const std::string& outputFile,
                         const std::string& blueconfig,
                         const std::string& report, const std::string& target)
{
    std::ofstream output;
    output.open(outputFile + "_volume_floats_" +
                    createTimeStepSuffix(time) + ".raw",
                std::ios::out | std::ios::binary);
    output.write((char*)_data.get(), sizeof(float) * _getVoxelCount());
    output.close();

    std::string voltUnit = dataUnit;
    std::replace(voltUnit.begin(), voltUnit.end(), 'A', 'V');

    std::ofstream info;
    info.open(outputFile + "_volume_info_" + createTimeStepSuffix(time) + ".txt");
    info << "# File generated by EMSim tool:\n"
         << "# - BlueConfig: " << blueconfig << "\n"
         << "# - Target: " << target << "\n"
         << "# - Report: " << report << "\n"
         << "# - Time step: " << timeStep << "\n"
         << "# - Units: " << voltUnit << "\n"
         << "# - SizeInVoxels: " << _volumeSize.x << " " << _volumeSize.y << " " << _volumeSize.z << "\n"
         << "# - SizeInMicrons: " << _volumeSize.x * _voxelSize.x << " " << _volumeSize.y * _voxelSize.y << " " << _volumeSize.z * _voxelSize.z << "\n"
         << "#" << std::endl;

    std::cout << "INFO: Volume for time " << createTimeStepSuffix(time) << " written to disk." << std::endl;
}

void Volume::writeToFileMhd(const float time,
                            const std::string& dataUnit, 
                            const std::string& outputFile)
{
    const std::string volumeFileName = outputFile + "_volume_floats" + createTimeStepSuffix(time) + ".raw";
    std::ofstream output;
    output.open(volumeFileName, std::ios::out | std::ios::binary);
    output.write((char*)_data.get(), sizeof(float) * _getVoxelCount());
    output.close();

    std::string voltUnit = dataUnit;
    std::replace(voltUnit.begin(), voltUnit.end(), 'A', 'V');

    std::ofstream mhdFile;
    mhdFile.open(outputFile + "_volume_floats_" + createTimeStepSuffix(time) + ".mhd");

    mhdFile << "ObjectType = Image\n"
            << "NDims = 3\n"
            << "BinaryData = True\n"
            << "BinaryDataByteOrderMSB = False\n"
            << "CompressedData = False\n"
            << "TransformMatrix = 1 0 0 0 1 0 0 0 1\n"
            << "Offset = 0 0 0\n"
            << "CenterOfRotation = 0 0 0\n"
            << "AnatomicalOrientation = 0 0 0\n"
            << "ElementSpacing = "<< _voxelSize.x << " " << _voxelSize.y << " " << _voxelSize.z << "\n"
            << "DimSize = "<< _volumeSize.x << " " << _volumeSize.y << " " << _volumeSize.z << "\n"
            << "ElementType = MET_FLOAT\n"
            << "ElementDataFile = " << volumeFileName << "\n"
            << std::endl;

    std::cout << "INFO: Volume .mhd for time: " << createTimeStepSuffix(time) << " written to disk." << std::endl;
}

const glm::uvec3& Volume::getSize() const
{
    return _volumeSize;
}

const glm::vec3& Volume::getOrigin() const
{
    return _origin;
}

const glm::vec3& Volume::getVoxelSize() const
{
    return _voxelSize;
}

float* Volume::getData()
{
    return _data.get();
}

const float* Volume::getData() const
{
    return _data.get();
}

uint64_t Volume::_getVoxelCount() const
{
    return (uint64_t)_volumeSize.x * (uint64_t)_volumeSize.y *
           (uint64_t)_volumeSize.z;
}
}
