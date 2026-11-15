#include "VersionHistory.h"
#include <algorithm>

namespace branchdocs::domain {

void VersionHistory::addVersion(Version v) { m_versions.push_back(std::move(v)); }

std::optional<Version> VersionHistory::getLatestVersion() const
{
    if (m_versions.empty())
        return std::nullopt;
    return m_versions.back();
}

bool VersionHistory::removeVersion(const std::string &id)
{
    std::vector<Version> kept;
    kept.reserve(m_versions.size());
    bool removed = false;

    for (const auto &v : m_versions) {
        if (v.id() == id) {
            removed = true;
        } else {
            kept.push_back(v);
        }
    }

    if (!removed)
        return false;

    m_versions = std::move(kept);
    return true;
}

const Version *VersionHistory::findById(const std::string &id) const noexcept
{
    for (const auto &v : m_versions)
        if (v.id() == id)
            return &v;
    return nullptr;
}

} // namespace branchdocs::domain