TEMPLATE = subdirs

SUBDIRS += \
  AdminRights \
  Chiya \
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

LayoutController.depends = AdminRights QtGlobalInput WinApiAdapter
Chiya.depends = LayoutController RunGuard AdminRights
