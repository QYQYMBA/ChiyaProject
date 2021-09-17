TEMPLATE = subdirs

SUBDIRS += \
  AdminRights \
  Chiya \
  LayoutController \
  RunGuard \
  QtGlobalInput

Chiya.subdir = src/Chiya
LayoutController.subdir  = src/LayoutController
RunGuard.subdir  = src/RunGuard
AdminRights.subdir  = src/AdminRights
QtGlobalInput.subdir = src/QtGlobalInput

LayoutController.depends = AdminRights QtGlobalInput
Chiya.depends = LayoutController RunGuard AdminRights
