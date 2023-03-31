<p align="center">
  <a href="https://github.com/morrazzzz/ESM-Engine/releases"><img src="https://img.shields.io/github/v/release/morrazzzz/ESM-Engine?style=flat-square" alt="GitHub release"></a>
  <a href="https://github.com/morrazzzz/ESM-Engine/blob/main/LICENSE"><img src="https://img.shields.io/github/license/morrazzzz/ESM-Engine?style=flat-square" alt="License"></a>
  <a href="https://github.com/morrazzzz/ESM-Engine/releases"><img src="https://img.shields.io/github/downloads/morrazzzz/ESM-Engine/total?style=flat-square" alt="Downloads"></a>
  <a href="https://github.com/morrazzzz/ESM-Engine"><img src="https://img.shields.io/github/repo-size/morrazzzz/ESM-Engine?style=flat-square" alt="Size"></a>
<p align="center">
  <a href="https://github.com/morrazzzz/ESM-Engine/commits/main"><img src="https://img.shields.io/github/last-commit/morrazzzz/ESM-Engine?style=flat-square" alt="Commit">
  <a href="https://github.com/morrazzzz/ESM-Engine/pulls"><img src="https://img.shields.io/github/issues-pr/morrazzzz/ESM-Engine?style=flat-square" alt="Pull requests">
<p align="center">
  <a href="https://discord.gg/D4CK5Vu6t3"><img src="https://img.shields.io/discord/1030545450564075594.svg?style=for-the-badge&label=DISCORD&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)" alt="Discord"></a>

![ESM-Engine](splash.png)

ESM-Engine
==========================
* Движок был основан на [XRay](https://github.com/mortany/xray), а точнее его модификации для новых Visual Studio.
* Спасибо проектам, чьи наработки были здесь использованы: [XRay OMP X64](https://github.com/xrLil-Batya/xray-omp-x64), [OGSR Engine](https://github.com/OGSR/OGSR-Engine/), [STCOP Engine x64](https://github.com/mortany), SO-XRay-Engine, [Anomaly Engine](https://github.com/morrazzzz/xray-monolith-152), TSMP Engine, [NSProjectX](https://github.com/Deathman00/NSProjectX), [XRay 1.6(abramcumner)](https://github.com/abramcumner/xray16), [XRay 1.5(abramcumner)](https://github.com/abramcumner/xray15), [SoC-DX10](https://github.com/morrey/STALKER-SOC-DX10)

## Разная информация:
* О проблемах можно сообщать [сюда](https://github.com/morrazzzz/ESM-Engine/issues)
* [Авторы/Contributors](https://github.com/morrazzzz/ESM-Engine/graphs/contributors) проекта
* [Описание изменённого(Wiki, пока там пусто! Там что то будет когда будет первый релиз)](https://github.com/morrazzzz/ESM-Engine/wiki)

## Инструкция по сборке:
* Требуется актуальная версия Visual Studio 2022(Не важен тип Visual Studio, можно любой, например Community)
* Открываете ESM Engine.sln, которых находится в папке trunk
* Выбираете Release(остальное пока не работает) и x86(Win32) или x64(НЕ Рекомендуется, ИМЕЮТСЯ БАГИ!!). Далее нажимаете на кнопку собрать

## Цели проекта:
* Исправление ошибок оригинального движка
* Добавление новых возможностей для разработчиков
* Улучшение графики
* Обновление некоторых компонентов движка
* Добавление рендеров из Зова Припяти
* Очистка движка от мусора

## Основные нововведения:
* Обновлен LuaJIT
* Исправление многих ошибок
* Более стабильный код
* Настройка некоторых параметров из config`ов
* Сборка движка под 2022 студией
* Некоторые фишки из Anomaly 
* Консоль из Зова Припяти
* Исправление предупреждений при сборке
* Поддержка моделей из Зова Припяти и SDK 0.7
* Фиксы утечек памяти

## Известные проблемы:
* Вылет при выходе из игры(ведётся наблюдение)
* Резкая смена погоды(на дин. погоде, на статич. такой проблемы нету)
* Баганный x64(Например: не генерируется stack trace)