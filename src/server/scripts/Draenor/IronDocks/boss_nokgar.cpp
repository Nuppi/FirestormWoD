////////////////////////////////////////////////////////////////////////////////
//
//  MILLENIUM-STUDIO
//  Copyright 2015 Millenium-studio SARL
//  All Rights Reserved.
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "iron_docks.h"
#include "Vehicle.h"

enum Yells
{
    // Nok'gar
    SAY_AGGRO = 0, // Start the funeral pyres! (46057)
    SAY_SLAY = 1, // Hahahaha my blade's thirst for war! (46061) / Revolt.. in the slaughter! (46060)
    SAY_DEATH = 2, // You.. will burn.. with me. (46058)
    SAY_INTRO = 3, // Warsong arrows sing from the skies, they'll be the last thing you hear as I crush your skull. (46059)
    SAY_SPELL01 = 4, // Let them have it! (46062)
    SAY_SPELL02 = 5, // That's it? you bearly drew blood! (46063)
    SAY_SPELL03 = 6, // Terror overwhelms you.. Death is near! (46064)
    SAY_SPELL04 = 7, // A death worthy of a warrior. (46065)

    // After Death
    TALK_ZOGGOSH_01 = 8, // Sir, they've breached the gates! Nok'gar is.. dead!! We should pick anchor and prepare to sail to Talador. Right now! (44047)
    TALK_ZOGGOSH_02 = 10, // But.. sir.. this is the last of the Grong that we have. Black hand will have our hides if we show up in the... (44048)
    TALK_ZOGGOSH_03 = 11, // Yes sir.. (44049)
    
    TALK_KORAMAR_01 = 23, // Calm yourself.. Zoggosh. We'll do no such thing, if these weaklings are so eager to die then we should obliged! (43899)

    TALK_KORAMAR_0222 = 24, // Zoggosh.. do not question my authority. This isn't just any Groon, this is Skuloc son of Grool.. the blood of a champion course through his veins.. I'm not concerned with these whelps. (43900)  
};

enum Spells
{
    SPELL_BARBED_ARROW = 164370,
    SPELL_BURNING_ARROW_DUMMY = 172810,
    SPELL_RECKLESS_PROVOCATION = 164426,
    SPELL_INITMIDATED = 164504,
    // Wolf - Dreadfang
    SPELL_BLOODLETTING_HOWL = 164835,
    SPELL_SAVAGE_MAULING = 154039,
    SPELL_SHERDDING_SWIPES_DOT = 164734,
    SPELL_SHREDDING_SWIPES_JUMP = 164735,
    SPELL_SHREDDING_SWIPES_DUMMY = 164730,
    SPELL_SHREDDING_SWIPES_AURA_TO_REMOVE = 164733,
    // Adds
    SPELL_BARBED_ARROW_AREA_TRIGGER = 164278,
    SPELL_BURNING_ARROW_AREA_TRIGGER = 164234,
    // MISC
    SPELL_WARSONG_FLAG = 168531,
};

enum Events
{
    EVENT_FIRE_ARROWS = 900,
    EVENT_FIRE_ARROWS_SIGNAL = 901,
    EVENT_BARBED_ARROWS = 902,
    EVENT_RECKLESS_PROVOCATION = 903,
    EVENT_DISMOUNT = 904,
    // Wolf
    EVENT_BLOODLETTING_HOWL = 905,
    EVENT_SAVAGE_MAULING = 906,
    EVENT_SHREDDING_SWIPES = 907,
    EVENT_SHREDDING_SWIPES_PART_2 = 908,
    EVENT_SHREDDING_SWIPES_PART_3 = 909,
    // 
    EVENT_FIRE_ARROWS_CHECK = 910,
};

enum Actions
{
    ACTION_FIRE_ARROWS = 999,
};

enum Vehicles
{
    CREATURE_WOLF = 81297,
};

enum Triggers
{
    SHREDDING_SWIPES_TRIGGER = 81832,
};

Position g_Archers[5] =
{
    { 6882.93f, -694.61f, 55.554f, 3.14270f },
    { 6883.21f, -705.07f, 55.922f, 3.13948f },
    { 6883.21f, -688.00f, 56.686f, 3.16305f },
    { 6884.02f, -676.18f, 56.483f, 3.34997f },
    { 6884.07f, -662.27f, 56.541f, 3.09864f },
};

class Nokgar_Death_Event : public BasicEvent
{
    public:
        explicit Nokgar_Death_Event(Unit* p_Unit, int p_Value) : m_Object(p_Unit), m_Modifier(p_Value) { }

        bool Execute(uint64 /*p_CurrTime*/, uint32 /*p_Diff*/)
        {
            if (InstanceScript* l_InstanceScript = m_Object->GetInstanceScript())
            {
                if (Creature* l_Zoggosh = l_InstanceScript->instance->GetCreature(l_InstanceScript->GetData64(DATA_ZUGGOSH)))
                {
                    if (Creature* l_Koramar = l_InstanceScript->instance->GetCreature(l_InstanceScript->GetData64(DATA_KORAMAR)))
                    {
                        if (Creature* l_Skulloc = l_InstanceScript->instance->GetCreature(l_InstanceScript->GetData64(DATA_SKULLOC)))
                        {
                            l_Skulloc->GetMap()->SetObjectVisibility(1000.0f);
                            l_Koramar->GetMap()->SetObjectVisibility(1000.0f);
                            l_Zoggosh->GetMap()->SetObjectVisibility(1000.0f);

                            switch (m_Modifier)
                            {
                                case 0:
                                    l_Zoggosh->AI()->Talk(TALK_ZOGGOSH_01);
                                    l_Zoggosh->m_Events.AddEvent(new Nokgar_Death_Event(l_Zoggosh, 1), l_Zoggosh->m_Events.CalculateTime(8000));
                                    break;
                                case 1:
                                    l_Koramar->AI()->Talk(TALK_KORAMAR_01);
                                    l_Koramar->m_Events.AddEvent(new Nokgar_Death_Event(l_Koramar, 2), l_Koramar->m_Events.CalculateTime(12000));
                                    break;
                                case 2:
                                    l_Koramar->AI()->Talk(TALK_KORAMAR_0222);
                                    break;
                            }
                        }
                    }
                }
            }

            return true;
        }

    private:
        Unit* m_Object;
        int m_Modifier;
};

#define dismountTimer 29000
#define recklessprovocationmsg "Fleshrender Nok'gar casts |cffff0000[Reckless Provocation]|cfffaeb00!"

/// Dreadfang <Fleshrender Nok'gar's Mount> - 81297
class boss_mount_wolf : public CreatureScript
{
    public:
        boss_mount_wolf() : CreatureScript("boss_mount_wolf") { }

        struct boss_mount_wolfAI : public BossAI
        {
            boss_mount_wolfAI(Creature* creature) : BossAI(creature, DATA_MOUNT_WOLF), vehicle(creature->GetVehicleKit())
            {
                me->Respawn(true);
                Reset();
            }

            Vehicle* vehicle;
            InstanceScript* pinstance = me->GetInstanceScript();
            uint32 diffforshreddingstrike;

            void Reset() override
            {
                _Reset();
                events.Reset();
                ASSERT(vehicle);

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DespawnCreaturesInArea(NPC_GROMKAR_FLAMESLINGER, me);
                me->SetReactState(REACT_AGGRESSIVE);
                diffforshreddingstrike = 300;
            }

            void DespawnCreaturesInArea(uint32 entry, WorldObject* object)
            {
                std::list<Creature*> creatures;
                GetCreatureListWithEntryInGrid(creatures, object, entry, 300.0f);
                if (creatures.empty())
                    return;

                for (std::list<Creature*>::iterator iter = creatures.begin(); iter != creatures.end(); ++iter)
                    (*iter)->DespawnOrUnsummon();
            }

            void EnterCombat(Unit* /*who*/) override
            {
                _EnterCombat();
                Talk(SAY_AGGRO);

                events.ScheduleEvent(EVENT_SAVAGE_MAULING, urand(10000, 15000));
                events.ScheduleEvent(EVENT_BLOODLETTING_HOWL, 20000);
                events.ScheduleEvent(EVENT_SHREDDING_SWIPES, urand(8000, 30000));
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void DamageTaken(Unit* attacker, uint32 &damage, SpellInfo const* p_SpellInfo)
            {
                if (me->GetHealthPct() <= 12)
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                    me->SetHealth(me->GetMaxHealth());
                    me->CastSpell(me, 103750); // feign death
                    me->CastSpell(me, 166925); // cosmetic feign death

                    if (Creature* wolf = pinstance->instance->GetCreature(pinstance->GetData64(DATA_MOUNT_WOLF)))
                    {
                        wolf->GetVehicleKit()->RemoveAllPassengers();
                    }

                    if (Creature* nokgar = pinstance->instance->GetCreature(pinstance->GetData64(DATA_NOKGAR)))
                        nokgar->AI()->Talk(SAY_SPELL04);
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
            }

            void MoveInLineOfSight(Unit* who)
            {
                if (who && who->IsInWorld() && who->GetEntry() == SHREDDING_SWIPES_TRIGGER && me->IsWithinDistInMap(who, 1.0f))
                {
                    who->ToCreature()->DespawnOrUnsummon();
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveAura(SPELL_SHREDDING_SWIPES_AURA_TO_REMOVE);
                }
            }

            void JustReachedHome()
            {
                me->DespawnOrUnsummon(1000);
                me->SummonCreature(me->GetEntry(), me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                // I think blizzard actually hacked it themselves aswell.
                if (me->HasAura(SPELL_SHREDDING_SWIPES_AURA_TO_REMOVE))
                {
                    if (diffforshreddingstrike <= diff)
                    {
                        std::list<Player*> PL_list;

                        JadeCore::AnyPlayerInObjectRangeCheck check(me, 3.0f);
                        JadeCore::PlayerListSearcher<JadeCore::AnyPlayerInObjectRangeCheck> searcher(me, PL_list, check);
                        me->VisitNearbyObject(3.0f, searcher);

                        if (PL_list.empty())
                            return;

                        for (std::list<Player*>::const_iterator itr = PL_list.begin(); itr != PL_list.end(); ++itr)
                        {
                            if (me->isInFront((*itr), M_PI * 0.5f))
                                (*itr)->AddAura(SPELL_SHREDDING_SWIPES_DUMMY, (*itr));
                        }

                        diffforshreddingstrike = 350;
                    }
                    else
                        diffforshreddingstrike -= diff;
                }

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_SAVAGE_MAULING:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        {
                            me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 8.0, 4.0f);
                            me->CastSpell(target, SPELL_SAVAGE_MAULING);

                            events.ScheduleEvent(EVENT_SAVAGE_MAULING, urand(10000, 15000));
                        }
                        break;
                    case EVENT_BLOODLETTING_HOWL:
                        me->CastSpell(me, SPELL_BLOODLETTING_HOWL);
                        events.ScheduleEvent(EVENT_BLOODLETTING_HOWL, 20000);
                        break;
                    case EVENT_SHREDDING_SWIPES:
                        me->AddAura(SPELL_SHREDDING_SWIPES_AURA_TO_REMOVE, me);
                        me->SetReactState(REACT_PASSIVE);

                        Position RandomLocationForTrigger;
                        me->GetRandomNearPosition(RandomLocationForTrigger, 20.0F);

                        me->SummonCreature(SHREDDING_SWIPES_TRIGGER, RandomLocationForTrigger, TEMPSUMMON_MANUAL_DESPAWN);
                        events.ScheduleEvent(EVENT_SHREDDING_SWIPES, urand(8000, 30000));
                        events.ScheduleEvent(EVENT_SHREDDING_SWIPES_PART_2, 500);
                        break;
                    case EVENT_SHREDDING_SWIPES_PART_2:
                        if (Creature* shredding_swipes_trigger = me->FindNearestCreature(SHREDDING_SWIPES_TRIGGER, 100.0F, true))
                        {
                            Position pos;
                            me->GetPosition(&pos);
                            me->GetMotionMaster()->MovePoint(0, pos);
                        }
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_mount_wolfAI(creature);
        }
};

/// Fleshrender Nok'gar - 81305
class boss_nokgar : public CreatureScript
{
    public:
        boss_nokgar() : CreatureScript("boss_nokgar") { }

        struct boss_nokgarAI : public BossAI
        {
            boss_nokgarAI(Creature* creature) : BossAI(creature, DATA_NOKGAR)
            {
                intro = false;
            }

            int8 phase;
            bool intro;
            bool dismountheroic;
            InstanceScript* pinstance = me->GetInstanceScript();

            void Reset() override
            {
                _Reset();
                events.Reset();
                summons.DespawnAll();
               // MountWolf();
                SummonArchers();

                me->SetReactState(REACT_AGGRESSIVE);
                phase = 0; // mounted
                me->CastSpell(me, SPELL_WARSONG_FLAG);
                dismountheroic = false;
            }

            void MountWolf()
            {
                if (Creature* wolf = instance->instance->GetCreature(instance->GetData64(DATA_MOUNT_WOLF)))
                {
                    //me->EnterVehicle(wolf, 0, true);
                   // wolf->Respawn(true);
                   // wolf->GetAI()->Reset();
                   // wolf->getThreatManager().resetAllAggro();
                   //wolf->GetVehicleKit()->AddPassenger(me, 0);
                   //me->_EnterVehicle(wolf->GetVehicleKit(), 0);
                   // me->AddUnitState(UNIT_STATE_ONVEHICLE);
                }
            }

            void SummonArchers()
            {
                for (int i = 0; i <= 5; i++)
                {
                   Creature* flameslinger = me->SummonCreature(NPC_GROMKAR_FLAMESLINGER, g_Archers[i], TEMPSUMMON_MANUAL_DESPAWN);
                  // if (flameslinger && flameslinger->IsInWorld())
                   //flameslinger->SetReactState(REACT_PASSIVE);
                }
            }

            void LaunchArchers()
            {
                std::list<Creature*> flameslinger;
                me->GetCreatureListWithEntryInGrid(flameslinger, NPC_GROMKAR_FLAMESLINGER, 100.0F);

                for (auto itr : flameslinger)
                {
                    itr->GetAI()->DoAction(ACTION_FIRE_ARROWS);
                }
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (who && who->IsInWorld() && who->GetTypeId() == TYPEID_PLAYER && !intro && me->IsWithinDistInMap(who, 10.0f))
                {
                    intro = true;
                    Talk(SAY_INTRO);
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage, SpellInfo const* p_SpellInfo) override
            {
                if (me->HealthBelowPct(50) && !dismountheroic)
                {
                    dismountheroic = true;
                    events.ScheduleEvent(EVENT_DISMOUNT, 1000);
                }
            }

            void EnterCombat(Unit* who) override
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                LaunchArchers();

                if (Creature* wolf = instance->instance->GetCreature(instance->GetData64(DATA_MOUNT_WOLF)))
                {
                    wolf->Attack(who, true);
                    me->Attack(who, true);
                }
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

                if (!me->GetMap()->IsHeroic())
                    events.ScheduleEvent(EVENT_DISMOUNT, dismountTimer);
            }

            void KilledUnit(Unit* who) override
            {
                if (who->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);

                me->m_Events.AddEvent(new Nokgar_Death_Event(me, 0), me->m_Events.CalculateTime(2000));
            }

            void JustReachedHome() override
            {
                if (Creature* wolf = pinstance->instance->GetCreature(pinstance->GetData64(DATA_MOUNT_WOLF)))
                {
                    wolf->DespawnOrUnsummon(1000);
                    me->DespawnOrUnsummon(1000);

                    me->SummonCreature(CREATURE_WOLF, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                }
            }

            void UpdateAI(uint32 const diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DISMOUNT:
                        {
                            if (Creature* wolf = instance->instance->GetCreature(instance->GetData64(DATA_MOUNT_WOLF)))
                            {
                                wolf->GetVehicleKit()->RemoveAllPassengers(true);
                            }

                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                            events.SetPhase(1); // non mounted
                            events.ScheduleEvent(EVENT_RECKLESS_PROVOCATION, 10000);
                            break;
                        }
                        case EVENT_RECKLESS_PROVOCATION:
                        {
                            Talk(SAY_SPELL03);
                            me->MonsterTextEmote(recklessprovocationmsg, me->GetGUID());
                            me->CastSpell(me, SPELL_RECKLESS_PROVOCATION);
                            events.ScheduleEvent(EVENT_RECKLESS_PROVOCATION, urand(15000, 20000));
                        }
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_nokgarAI(creature);
        }
};

class iron_docks_flameslinger : public CreatureScript
{
    public:
        iron_docks_flameslinger() : CreatureScript("iron_docks_flameslinger") { }

        struct mob_iron_docksAI : public ScriptedAI
        {
            mob_iron_docksAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetDisableGravity(true);
                me->SetHover(true);
            }
        
            bool signal;
            int32 dshot;
            int32 eshot;
            InstanceScript* pinstance = me->GetInstanceScript();

            void Reset()
            {
                //signal = true;

                dshot = 0;
                eshot = 0;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->setFaction(16);
                me->RemoveAllAuras();
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                case ACTION_FIRE_ARROWS:
                    //signal = true;

                   // events.ScheduleEvent(EVENT_FIRE_ARROWS_SIGNAL, 1000);
                    events.ScheduleEvent(EVENT_FIRE_ARROWS, 2000);
                    events.ScheduleEvent(EVENT_BARBED_ARROWS, 12000);
                    break;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_FIRE_ARROWS:
                        if (Player* target = me->FindNearestPlayer(200.0f, true))
                       // if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0F, true))
                            me->CastSpell(target, SPELL_BURNING_ARROW_AREA_TRIGGER, true);
                       // me->CastSpell(me, SPELL_BURNING_ARROW_DUMMY);
                        events.ScheduleEvent(EVENT_FIRE_ARROWS, 4000);

                        dshot++;
                        if (dshot >= 4)
                        {
                            dshot = 0;
                            events.CancelEvent(EVENT_FIRE_ARROWS);
                            events.ScheduleEvent(EVENT_FIRE_ARROWS_CHECK, 1000);
                        }
                        break;
                    case EVENT_BARBED_ARROWS:
                        if (Player* target = me->FindNearestPlayer(200.0f, true))
                            me->CastSpell(target, SPELL_BARBED_ARROW_AREA_TRIGGER, true);
                        //me->CastSpell(me, SPELL_BARBED_ARROW);
                        events.ScheduleEvent(SPELL_BARBED_ARROW, 6000);
                        eshot++;

                        if (eshot >= 4)
                        {
                            eshot = 0;
                            events.CancelEvent(EVENT_BARBED_ARROWS);
                            events.ScheduleEvent(EVENT_FIRE_ARROWS_CHECK, 1000);
                        }
                        break;
                    case EVENT_FIRE_ARROWS_CHECK:
                        eshot = 0;
                        dshot = 0;
                        events.ScheduleEvent(SPELL_BARBED_ARROW, 20000);
                        events.ScheduleEvent(EVENT_FIRE_ARROWS, 20000);
                        break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_iron_docksAI(creature);
        }
};

/// Reckless Provocation - 164426
class spell_irondock_reckless_provocation : public SpellScriptLoader
{
    public:
        spell_irondock_reckless_provocation() : SpellScriptLoader("spell_irondock_reckless_provocation") { }

        class spell_irondock_reckless_provocation_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_irondock_reckless_provocation_AuraScript);

            void OnProc(constAuraEffectPtr p_AurEff, ProcEventInfo& p_EventInfo)
            {
                PreventDefaultAction();

                if (Unit* l_Caster = GetCaster())
                {
                    if (Unit* l_Target = p_EventInfo.GetActor())
                        l_Caster->CastSpell(l_Target, GetSpellInfo()->Effects[0].TriggerSpell, true);
                }
            }

            void Register()
            {
                OnEffectProc += AuraEffectProcFn(spell_irondock_reckless_provocation_AuraScript::OnProc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_irondock_reckless_provocation_AuraScript();
        }
};

void AddSC_boss_nokgar()
{
    new boss_nokgar();
    new iron_docks_flameslinger();
    new boss_mount_wolf();
    new spell_irondock_reckless_provocation();
}