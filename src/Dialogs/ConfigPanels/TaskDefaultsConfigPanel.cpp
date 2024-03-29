/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Listener.hpp"
#include "Interface.hpp"
#include "Dialogs/dlgTaskHelpers.hpp"
#include "Task/Factory/AbstractTaskFactory.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include "Form/RowFormWidget.hpp"
#include "TaskDefaultsConfigPanel.hpp"
#include "UIGlobals.hpp"

enum ControlIndex {
  StartType,
  StartRadius,
  spacer_1,
  FinishType,
  FinishRadius,
  spacer_2,
  TurnpointType,
  TurnpointRadius,
  spacer_3,
  TaskType,
  AATMinTime,
  AATTimeMargin
};

class TaskDefaultsConfigPanel
  : public RowFormWidget, DataFieldListener {
public:
  TaskDefaultsConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  void SetStartLabel();
  void SetFinishLabel();

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df);
};

void
TaskDefaultsConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(StartType, df))
    SetStartLabel();
  else if (IsDataField(FinishType, df))
    SetFinishLabel();
}

static const TCHAR *Caption_GateWidth = N_("Gate width");
static const TCHAR *Caption_Radius = N_("Radius");

void
TaskDefaultsConfigPanel::SetStartLabel()
{
  WndProperty &wp = GetControl(StartRadius);

  if (GetValueInteger(StartType) == (int)TaskPointFactoryType::START_LINE)
    wp.SetCaption(gettext(Caption_GateWidth));
  else
    wp.SetCaption(gettext(Caption_Radius));
}

void
TaskDefaultsConfigPanel::SetFinishLabel()
{
  WndProperty &wp = GetControl(FinishRadius);

  if (GetValueInteger(FinishType) == (int)TaskPointFactoryType::FINISH_LINE)
    wp.SetCaption(gettext(Caption_GateWidth));
  else
    wp.SetCaption(gettext(Caption_Radius));
}


void
TaskDefaultsConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  WndProperty *wp;
  const ComputerSettings &settings_computer = XCSoarInterface::GetComputerSettings();
  const TaskBehaviour &task_behaviour = settings_computer.task;
  OrderedTask temptask(task_behaviour);
  temptask.SetFactory(TaskFactoryType::RACING);

  RowFormWidget::Prepare(parent, rc);

  wp = AddEnum(_("Start point"),
               _("Default start type for new tasks you create."),
               this);
  if (wp) {
    const auto point_types = temptask.GetFactory().GetValidStartTypes();
    DataFieldEnum* dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->EnableItemHelp(true);

    for (auto i = point_types.begin(), end = point_types.end();
         i != end; ++i) {
      const TaskPointFactoryType type = *i;
      dfe->addEnumText(OrderedTaskPointName(type), (unsigned)type,
                       OrderedTaskPointDescription(type));
      if (type == task_behaviour.sector_defaults.start_type)
        dfe->Set((unsigned)type);
    }
    wp->RefreshDisplay();
  }

  AddFloat(Caption_GateWidth, _("Default radius or gate width of the start zone for new tasks."),
           _T("%.1f %s"), _T("%.1f"), fixed(0.1), fixed(100), fixed(1.0), true, UnitGroup::DISTANCE,
           task_behaviour.sector_defaults.start_radius);

  AddSpacer();

  wp = AddEnum(_("Finish point"),
               _("Default finish type for new tasks you create."),
               this);
  if (wp) {
    const auto point_types = temptask.GetFactory().GetValidFinishTypes();
    DataFieldEnum* dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->EnableItemHelp(true);

    for (auto i = point_types.begin(), end = point_types.end();
         i != end; ++i) {
      const TaskPointFactoryType type = *i;
      dfe->addEnumText(OrderedTaskPointName(type), (unsigned)type,
                       OrderedTaskPointDescription(type));
      if (type == task_behaviour.sector_defaults.finish_type)
        dfe->Set((unsigned)type);
    }
    wp->RefreshDisplay();
  }

  AddFloat(Caption_GateWidth, _("Default radius or gate width of the finish zone in new tasks."),
           _T("%.1f %s"), _T("%.1f"), fixed(0.1), fixed(100), fixed(1.0), true, UnitGroup::DISTANCE,
           task_behaviour.sector_defaults.finish_radius);

  AddSpacer();

  wp = AddEnum(_("Turn point"), _("Default turn point type for new tasks you create."));
  if (wp) {
    const auto point_types = temptask.GetFactory().GetValidIntermediateTypes();
    DataFieldEnum* dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->EnableItemHelp(true);

    for (auto i = point_types.begin(), end = point_types.end();
         i != end; ++i) {
      const TaskPointFactoryType type = *i;
      dfe->addEnumText(OrderedTaskPointName(type), (unsigned)type,
                       OrderedTaskPointDescription(type));
      if (type == task_behaviour.sector_defaults.turnpoint_type) {
        dfe->Set((unsigned)type);
      }
    }
    wp->RefreshDisplay();
  }

  AddFloat(Caption_Radius, _("Default radius of turnpoint cylinders and sectors in new tasks."),
           _T("%.1f %s"), _T("%.1f"), fixed(0.1), fixed(100), fixed(1.0), true, UnitGroup::DISTANCE,
           task_behaviour.sector_defaults.turnpoint_radius);

  AddSpacer();

  wp = AddEnum(_("Task"), _("Default task type for new tasks you create."));
  if (wp) {
    const std::vector<TaskFactoryType> factory_types =
      temptask.GetFactoryTypes();
    DataFieldEnum* dfe = (DataFieldEnum*)wp->GetDataField();
    dfe->EnableItemHelp(true);

    for (unsigned i = 0; i < factory_types.size(); i++) {
      dfe->addEnumText(OrderedTaskFactoryName(factory_types[i]),
          (unsigned)factory_types[i], OrderedTaskFactoryDescription(
              factory_types[i]));
      if (factory_types[i] == task_behaviour.task_type_default)
        dfe->Set((unsigned)factory_types[i]);
    }
    wp->RefreshDisplay();
  }

  AddTime(_("AAT min. time"), _("Default AAT min. time for new AAT tasks."),
          60, 10 * 60 * 60, 60, (unsigned)task_behaviour.ordered_defaults.aat_min_time);

  AddTime(_("Optimisation margin"),
          _("Safety margin for AAT task optimisation.  Optimisation "
            "seeks to complete the task at the minimum time plus this margin time."),
          0, 30 * 60, 60, (unsigned)task_behaviour.optimise_targets_margin);
  SetExpertRow(AATTimeMargin);

  SetStartLabel();
  SetFinishLabel();
}

bool
TaskDefaultsConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  ComputerSettings &settings_computer = XCSoarInterface::SetComputerSettings();
  TaskBehaviour &task_behaviour = settings_computer.task;

  changed |= SaveValueEnum(StartType, szProfileStartType, task_behaviour.sector_defaults.start_type);

  changed |= SaveValue(StartRadius, UnitGroup::DISTANCE, szProfileStartRadius,
                       task_behaviour.sector_defaults.start_radius);

  changed |= SaveValueEnum(TurnpointType, szProfileTurnpointType,
                           task_behaviour.sector_defaults.turnpoint_type);

  changed |= SaveValue(TurnpointRadius, UnitGroup::DISTANCE, szProfileTurnpointRadius,
                       task_behaviour.sector_defaults.turnpoint_radius);

  changed |= SaveValueEnum(FinishType, szProfileFinishType,
                           task_behaviour.sector_defaults.finish_type);

  changed |= SaveValue(FinishRadius, UnitGroup::DISTANCE, szProfileFinishRadius,
                       task_behaviour.sector_defaults.finish_radius);

  changed |= SaveValueEnum(TaskType, szProfileTaskType, task_behaviour.task_type_default);

  unsigned aatminutes = (unsigned)task_behaviour.ordered_defaults.aat_min_time;
  if (SaveValue(AATMinTime, aatminutes)) {
    task_behaviour.ordered_defaults.aat_min_time = fixed(aatminutes);
    Profile::Set(szProfileAATMinTime, aatminutes);
    changed = true;
  }

  unsigned aatmargin = task_behaviour.optimise_targets_margin;
  if (SaveValue(AATTimeMargin, aatmargin)) {
    task_behaviour.optimise_targets_margin = aatmargin;
    Profile::Set(szProfileAATTimeMargin, aatmargin);
    changed = true;
  }

  _changed |= changed;
  _require_restart |= require_restart;
  return true;
}

Widget *
CreateTaskDefaultsConfigPanel()
{
  return new TaskDefaultsConfigPanel();
}
