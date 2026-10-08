# Sensirion Gas Index Algorithm

Официальный C core из https://github.com/Sensirion/arduino-gas-index-algorithm,
commit `e18754329cb8454589d36123be3e557b8a91359d`, без изменений.
Лицензия BSD-3-Clause сохранена в LICENSE и заголовках.

C API вместо Arduino wrapper VOCGasIndexAlgorithm вычисляет тот же VOC Index,
но два независимых состояния хранятся статически, без new/malloc.
Период — 1 секунда. После ошибки/пропуска reset запускает новый blackout.
