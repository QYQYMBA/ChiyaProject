TEMPLATE = subdirs

SUBDIRS += \
  AdminRights \
  Chiya \
  LayoutController \
  RunGuard

Chiya.subdir = src/Chiya
LayoutController.subdir  = src/LayoutController
RunGuard.subdir  = src/RunGuard
AdminRights.subdir  = src/AdminRights

LayoutController.depends = AdminRights
Chiya.depends = LayoutController RunGuard AdminRights
