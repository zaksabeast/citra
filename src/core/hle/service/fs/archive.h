// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/container/flat_map.hpp>
#include "common/common_types.h"
#include "core/file_sys/archive_backend.h"
#include "core/hle/result.h"
#include "core/hle/service/fs/directory.h"
#include "core/hle/service/fs/file.h"

/// The unique system identifier hash, also known as ID0
static constexpr char SYSTEM_ID[]{"00000000000000000000000000000000"};
/// The scrambled SD card CID, also known as ID1
static constexpr char SDCARD_ID[]{"00000000000000000000000000000000"};

namespace Loader {
class AppLoader;
}

namespace Service::FS {

/// Supported archive types
enum class ArchiveIdCode : u32 {
    SelfNCCH = 0x00000003,
    SaveData = 0x00000004,
    ExtSaveData = 0x00000006,
    SharedExtSaveData = 0x00000007,
    SystemSaveData = 0x00000008,
    SDMC = 0x00000009,
    SDMCWriteOnly = 0x0000000A,
    NCCH = 0x2345678A,
    OtherSaveDataGeneral = 0x567890B2,
    OtherSaveDataPermitted = 0x567890B4,
};

/// Media types for the archives
enum class MediaType : u32 { NAND = 0, SDMC = 1, GameCard = 2 };

typedef u64 ArchiveHandle;

using FileSys::ArchiveBackend;
using FileSys::ArchiveFactory;

class ArchiveManager {
public:
    ArchiveManager();
    /**
     * Opens an archive
     * @param id_code IdCode of the archive to open
     * @param archive_path Path to the archive, used with Binary paths
     * @return Handle to the opened archive
     */
    ResultVal<ArchiveHandle> OpenArchive(ArchiveIdCode id_code, FileSys::Path& archive_path);

    /**
     * Closes an archive
     * @param handle Handle to the archive to close
     */
    ResultCode CloseArchive(ArchiveHandle handle);

    /**
     * Open a File from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @param mode Mode under which to open the File
     * @return The opened File object
     */
    ResultVal<std::shared_ptr<File>> OpenFileFromArchive(ArchiveHandle archive_handle,
                                                         const FileSys::Path& path,
                                                         const FileSys::Mode mode);

    /**
     * Delete a File from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteFileFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Rename a File between two Archives
     * @param src_archive_handle Handle to the source Archive object
     * @param src_path Path to the File inside of the source Archive
     * @param dest_archive_handle Handle to the destination Archive object
     * @param dest_path Path to the File inside of the destination Archive
     * @return Whether rename succeeded
     */
    ResultCode RenameFileBetweenArchives(ArchiveHandle src_archive_handle,
                                         const FileSys::Path& src_path,
                                         ArchiveHandle dest_archive_handle,
                                         const FileSys::Path& dest_path);

    /**
     * Delete a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteDirectoryFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Delete a Directory and anything under it from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteDirectoryRecursivelyFromArchive(ArchiveHandle archive_handle,
                                                     const FileSys::Path& path);

    /**
     * Create a File in an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @param file_size The size of the new file, filled with zeroes
     * @return File creation result code
     */
    ResultCode CreateFileInArchive(ArchiveHandle archive_handle, const FileSys::Path& path,
                                   u64 file_size);

    /**
     * Create a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether creation of directory succeeded
     */
    ResultCode CreateDirectoryFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Rename a Directory between two Archives
     * @param src_archive_handle Handle to the source Archive object
     * @param src_path Path to the Directory inside of the source Archive
     * @param dest_archive_handle Handle to the destination Archive object
     * @param dest_path Path to the Directory inside of the destination Archive
     * @return Whether rename succeeded
     */
    ResultCode RenameDirectoryBetweenArchives(ArchiveHandle src_archive_handle,
                                              const FileSys::Path& src_path,
                                              ArchiveHandle dest_archive_handle,
                                              const FileSys::Path& dest_path);

    /**
     * Open a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return The opened Directory object
     */
    ResultVal<std::shared_ptr<Directory>> OpenDirectoryFromArchive(ArchiveHandle archive_handle,
                                                                   const FileSys::Path& path);

    /**
     * Get the free space in an Archive
     * @param archive_handle Handle to an open Archive object
     * @return The number of free bytes in the archive
     */
    ResultVal<u64> GetFreeBytesInArchive(ArchiveHandle archive_handle);

    /**
     * Erases the contents of the physical folder that contains the archive
     * identified by the specified id code and path
     * @param id_code The id of the archive to format
     * @param format_info Format information about the new archive
     * @param path The path to the archive, if relevant.
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode FormatArchive(ArchiveIdCode id_code, const FileSys::ArchiveFormatInfo& format_info,
                             const FileSys::Path& path = FileSys::Path());

    /**
     * Retrieves the format info about the archive of the specified type and path.
     * The format info is supplied by the client code when creating archives.
     * @param id_code The id of the archive
     * @param archive_path The path of the archive, if relevant
     * @return The format info of the archive, or the corresponding error code if failed.
     */
    ResultVal<FileSys::ArchiveFormatInfo> GetArchiveFormatInfo(ArchiveIdCode id_code,
                                                               FileSys::Path& archive_path);

    /**
     * Creates a blank SharedExtSaveData archive for the specified extdata ID
     * @param media_type The media type of the archive to create (NAND / SDMC)
     * @param high The high word of the extdata id to create
     * @param low The low word of the extdata id to create
     * @param smdh_icon the SMDH icon for this ExtSaveData
     * @param format_info Format information about the new archive
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode CreateExtSaveData(MediaType media_type, u32 high, u32 low,
                                 const std::vector<u8>& smdh_icon,
                                 const FileSys::ArchiveFormatInfo& format_info);

    /**
     * Deletes the SharedExtSaveData archive for the specified extdata ID
     * @param media_type The media type of the archive to delete (NAND / SDMC)
     * @param high The high word of the extdata id to delete
     * @param low The low word of the extdata id to delete
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode DeleteExtSaveData(MediaType media_type, u32 high, u32 low);

    /**
     * Deletes the SystemSaveData archive folder for the specified save data id
     * @param high The high word of the SystemSaveData archive to delete
     * @param low The low word of the SystemSaveData archive to delete
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode DeleteSystemSaveData(u32 high, u32 low);

    /**
     * Creates the SystemSaveData archive folder for the specified save data id
     * @param high The high word of the SystemSaveData archive to create
     * @param low The low word of the SystemSaveData archive to create
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode CreateSystemSaveData(u32 high, u32 low);

    /// Registers a new NCCH file with the SelfNCCH archive factory
    void RegisterSelfNCCH(Loader::AppLoader& app_loader);

private:
    /**
     * Registers an Archive type, instances of which can later be opened using its IdCode.
     * @param factory File system backend interface to the archive
     * @param id_code Id code used to access this type of archive
     */
    ResultCode RegisterArchiveType(std::unique_ptr<FileSys::ArchiveFactory>&& factory,
                                   ArchiveIdCode id_code);

    /// Register all archive types
    void RegisterArchiveTypes();

    ArchiveBackend* GetArchive(ArchiveHandle handle);

    /**
     * Map of registered archives, identified by id code. Once an archive is registered here, it is
     * never removed until UnregisterArchiveTypes is called.
     */
    boost::container::flat_map<ArchiveIdCode, std::unique_ptr<ArchiveFactory>> id_code_map;

    /**
     * Map of active archive handles to archive objects
     */
    std::unordered_map<ArchiveHandle, std::unique_ptr<ArchiveBackend>> handle_map;
    ArchiveHandle next_handle = 1;
};

} // namespace Service::FS
