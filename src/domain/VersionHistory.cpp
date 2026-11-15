#include "VersionHistory.h"

namespace branchdocs::domain {

void VersionHistory::addVersion(Version v) {
    m_versions.push_back(std::move(v));
}

std::optional<Version> VersionHistory::getLatestVersion() const {
    if (m_versions.empty()) return std::nullopt;
    return m_versions.back();
}

} // namespace branchdocs::domain
