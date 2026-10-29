# Встраиваемые системы — занятия 1.1–3.1

Решения заданий [курса ant-lab](https://es.ant-lab.ru/).
Источник заданий: [es-book](https://github.com/ant-lab-ru/es-book),
Актуальная версия книги получена 29 октября 2026 года.

**Программная часть подготовлена до 3.1.5 включительно.**
[Что выполнено и что требует платы](docs/available-2026-10-29.md) ·
[План подключения и снятия показаний](docs/capture-plan.md).

| Задания | Проект | Результат |
| --- | --- | --- |
| 1.1.1–1.1.5 | `112-blink`, `tools.log`, `.github/workflows` | Инструменты, мигание 250/1000 мс, сборка, Git и проверки |
| 1.2.1 | `121-blink-reg` | Управление LED через регистры SIO |
| 1.2.2 | `122-led-button` | Переключение LED кнопкой GP15 без фильтрации дребезга |
| 1.2.3 | `123-led-button-debounce` | Подавление дребезга, одно переключение на нажатие |
| 1.3.1–1.3.2 | `131-hello-usb` | USB CDC и скрипты регистрации Hello, world! |
| 1.3.3–1.3.4 | `133-led-button-usb` | Кнопка, сообщения LED и команды `e`, `d` |
| 1.3.5–1.3.7 | `134-led-module` | Модули LED, журнала и паспорта устройства |
| 2.1.1–2.1.6 | `211-command-usb` | Строковые команды, таблица обработчиков и диагностика памяти |
| 2.2.1–2.2.5 | `221-command-time` | Частоты, неблокирующее мигание, расчёт π, профилирование и смена частоты ядра |
| 2.3.1–2.3.4, неделя 7 | `231-firmware-template` | Протокол, API, задачи LED и кнопки, неблокирующий расчёт π |
| 2.3.5–2.3.8 | `231-firmware-template`, `libs` | Общие библиотеки, секундомеры, граница SDK, скрипт приёмки |
| 2.3.9, 2.3.9д | `239-two-buttons` | Две независимые кнопки и долгое нажатие |
| 3.1.1–3.1.5 | `311-adc` | АЦП, температура, телеметрия, запись графиков и RC-опыт |

[![2.3.1](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-1.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-1.yml)
[![2.3.2](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-2.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-2.yml)
[![2.3.3](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-3.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-3.yml)
[![2.3.4](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-4.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-4.yml)

Неделя 7 выполнена в объёме первой недели занятия 2.3: задания 2.3.1–2.3.4.
Остальные задания занятия относятся ко второй неделе. [Пояснения и проверка](docs/week-7.md).

[![2.2.1](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-1.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-1.yml)
[![2.2.2](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-2.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-2.yml)
[![2.2.3](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-3.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-3.yml)
[![2.2.4](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-4.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-4.yml)
[![2.2.5](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-5.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-2-5.yml)

## Прогон занятия 2.2 на плате

22 октября 2026 года прошивка проверена на настоящей Pico
с серийным номером `E66098F29B56BD32`. Все пять логов
`device-2-2-1.log`…`device-2-2-5.log` сохранены в `221-command-time`.
Расчёт π занял 4698 мс на 125 МГц и 9397 мс на 62,5 МГц;
после возврата частоты — снова 4698 мс. Аппаратные часы сохранили темп.
Порядок и пояснения: [docs/lesson-2-2.md](docs/lesson-2-2.md).

## Сборка

Приёмка **2.3.8** записана 29 октября 2026 года с настоящей Pico
`E66098F29B56BD32`: 36 кадров и 94 строки ответа в
`231-firmware-template/device-2-3-8.log`. Локальная проверка обмена и кода —
48/48; миллион членов ряда π рассчитан за 5107 мс, максимальная порция — 221 мкс.

Используются Pico SDK **2.3.0** и picotool **2.3.0**.
Фактические версии остальных установленных инструментов записаны в `tools.log`.

В подготовленной локальной рабочей папке Windows:

```powershell
. ./tools/enter-environment.ps1
./tools/build.ps1
# Один проект:
./tools/build.ps1 -Project 134-led-module
# Проект четвёртой недели:
./tools/build.ps1 -Project 211-command-usb
# Проект шестой недели:
./tools/build.ps1 -Project 221-command-time
# Проект седьмой недели:
./tools/build.ps1 -Project 231-firmware-template
# Сравнение размера с отключёнными отладочными сообщениями:
./tools/build.ps1 -Project 134-led-module -LogLevel 2
```

Скрипт `enter-environment.ps1` на этом компьютере использует инструменты
из соседней папки `work`. На другом компьютере установите инструменты,
задайте `PICO_SDK_PATH`, добавьте их в PATH и собирайте стандартно:

```powershell
cmake -S 112-blink -B 112-blink/build -G Ninja -Dpicotool_DIR=C:/path/to/picotool
cmake --build 112-blink/build
```

Результаты — `.elf`, `.bin`, `.uf2` в `build` внутри проекта.
Сборочные файлы исключены из Git. По курсу используется **обычная Pico на RP2040**:
встроенный светодиод подключён к GP25. Для Pico W этот код не подходит без адаптации.

## Проверка на плате и сдача

Подробный порядок: [docs/hardware-and-submission.md](docs/hardware-and-submission.md).

Файлы `.github/workflows/check-*.yml` вызывают официальные проверки
[ant-lab-ru/es-test](https://github.com/ant-lab-ru/es-test).
При каждом `git push` GitHub Actions собирает проекты и проверяет исходники и логи.
Секрет `ES_ACCOUNT_SECRET` хранится только в настройках GitHub Actions:
он позволяет официальной проверке отправлять результаты в личный кабинет курса.

## Решения и пояснения

- При одновременном событии кнопки и USB-команде приоритет имеет USB:
  кнопку обрабатываем первой, затем явную команду компьютера.
- Дребезг подавляется после **20 мс непрерывно стабильного уровня** на обоих фронтах.
  В `133-led-button-usb` выдержка увеличена до **200 мс** для ручного замыкания
  проводом: при аппаратном прогоне обнаружены повторные переключения через 86–159 мс.
  Для этого проекта удерживайте замыкание около секунды и полностью размыкайте контакт.
  Это уточнение учебного примера с двумя чтениями. Таймер не блокирует обработку USB;
  суперцикл делает паузу 1 мс между опросами. Функция рассчитана на одну кнопку.
- Нажатие определяется переходом высокого уровня в низкий. Удержание не повторяет событие.
- Команды `e` и `d` всегда подтверждаются, в том числе при повторной команде.
- `v` сообщает версию прошивки, `i` — паспорт платы. Номер платы и CHIP_ID читаются из железа.
- Проект `211-command-usb` принимает слова `enable`, `disable`, `info`, `version`,
  `ping`, `mem_info`, `fw_info`, `dev_info` и `boot_info` с переводом строки.
- В `221-command-time` добавлены `clk_info`, `uptime`, `calc_pi`, `main_time_exec`,
  `main_time_reset`, `clk_sys_low` и `clk_sys_default`. Мигание не задерживает опрос USB.
- [Ответы на контрольные вопросы](docs/answers.md).

Скрипты опроса USB, файлы подключения SDK и правила вызова проверок основаны на
официальных материалах курса. Лицензия материалов курса сохранена в `LICENSE.course`.

## Проверки следующих заданий

Приёмка и измерительные задания запускаются после добавления настоящих файлов опыта.

[![2.3.5](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-5.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-5.yml)
[![2.3.6](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-6.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-6.yml)
[![2.3.7](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-7.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-7.yml)
[![2.3.8](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-8.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-8.yml)
[![2.3.9](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-9.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-9.yml)
[![2.3.9d](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-9d.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-2-3-9d.yml)
[![3.1.1](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-1.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-1.yml)
[![3.1.2](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-2.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-2.yml)
[![3.1.3](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-3.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-3.yml)
[![3.1.4](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-4.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-4.yml)
[![3.1.5](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-5.yml/badge.svg)](https://github.com/levsasha2018-crypto/RT/actions/workflows/check-3-1-5.yml)
