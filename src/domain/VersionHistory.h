#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "Version.h"

namespace branchdocs::domain {

class VersionHistory {
public:
    void addVersion(Version v);
    std::optional<Version> getLatestVersion() const;
    const std::vector<Version>& getAllVersions() const noexcept { return m_versions; }
    std::size_t size() const noexcept { return m_versions.size(); }

private:
    std::vector<Version> m_versions;
};

} // namespace branchdocs::domain
