#pragma once
#include "megdnn/basic_types.h"
#include "megdnn/handle.h"

namespace megdnn {
namespace test {

class WorkspaceWrapper {
public:
    WorkspaceWrapper();
    WorkspaceWrapper(Handle* handle, size_t size_in_bytes = 0);
    ~WorkspaceWrapper();

    WorkspaceWrapper& operator=(const WorkspaceWrapper& wrapper);
    void clear();
    void update(size_t size_in_bytes);

    bool valid() const { return m_handle != nullptr; }
    Workspace workspace() const { return m_workspace; }

private:
    Handle* m_handle;
    Workspace m_workspace;
};

}  // namespace test
}  // namespace megdnn
// vim: syntax=cpp.doxygen
