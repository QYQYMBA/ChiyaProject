TEMPLATE = subdirs

SUBDIRS += \
  AdminRights \
  Chiya \
  CorrectLayout \
  LayoutController \
  RunGuard \
  QtGlobalInput \
  WinApiAdapter

Chiya.subdir = src/Chiya
LayoutController.subdir  = src/LayoutController
RunGuard.subdir  = src/RunGuard
AdminRights.subdir  = src/AdminRights
QtGlobalInput.subdir = src/QtGlobalInput
WinApiAdapter.subdir = src/WinApiAdapter
CorrectLayout.subdir = src/CorrectLayout

LayoutController.depends = AdminRights QtGlobalInput WinApiAdapter
Chiya.depends = CorrectLayout LayoutController RunGuard AdminRights
CorrectLayout.depends = QtGlobalInput WinApiAdapter LayoutController
