# Difficulty+

A [Satisfactory](https://www.satisfactorygame.com/) mod that makes the Space Elevator harder.

> **EXPERIMENTAL (v0.1.0).** Early version, not tested in a full playthrough. Back up your saves.

## Short description

Choose how much each Space Elevator phase costs when you create a world.

## Description

Difficulty+ makes the Space Elevator project assembly phases as hard as you want. When you create a new world, open **Mod Savegame Settings** and pick a cost multiplier for each phase: x0.5, x1, x2, x3, x5 or x10. Every option lists the resulting item counts, so you see exactly what each phase will cost before you commit.

- Settings are fixed at world creation and hidden in-game.
- Costs scale from vanilla values, rounded up (minimum 1 per item).
- Each world keeps its own values.

### Known limitations

- Long option text may be cut off in the menu.
- Multiplayer: clients joining a game may still see the vanilla cost in some screens. Everyone should have the mod installed.

## Requirements

- Satisfactory build `>=491125`
- [SML](https://ficsit.app/mod/SML) `^3.12.0`

## Building

This is a code-only mod. Drop the folder into `Mods/` of the
[Satisfactory starter project](https://github.com/satisfactorymodding/SatisfactoryModLoader)
(Unreal Engine `5.6.1-CSS`) and package it with Alpakit. The mod reference is `SpaceElevatorTuner`.
