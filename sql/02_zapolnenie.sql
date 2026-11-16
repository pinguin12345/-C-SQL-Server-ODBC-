-- Учебные данные. Скрипт ничего не удаляет:
-- строки добавляются, только если таблицы еще пустые.
-- Свои отделы и сотрудников можно вводить через меню программы.
USE OtpuskKompanii;
GO

IF NOT EXISTS (SELECT 1 FROM Otdely)
   AND NOT EXISTS (SELECT 1 FROM Sotrudniki)
   AND NOT EXISTS (SELECT 1 FROM Otpuska)
BEGIN
    INSERT INTO Otdely (Id, Nazvanie) VALUES
        (1, N'Бухгалтерия'),
        (2, N'Отдел продаж'),
        (3, N'Склад');

    INSERT INTO Sotrudniki (Id, Fio, Tabel, OtdelId, Norma) VALUES
        (1, N'Иванов Иван Иванович', N'1001', 1, 28),
        (2, N'Петрова Анна Сергеевна', N'1002', 1, 28),
        (3, N'Сидоров Петр Алексеевич', N'1003', 2, 28),
        (4, N'Кузнецова Мария Олеговна', N'1004', 2, 28),
        (5, N'Волкова Елена Игоревна', N'1005', 3, 28);

    INSERT INTO Otpuska (Id, SotrId, DataNachala, DataKonca, Dney) VALUES
        (1, 2, N'01.06.2027', N'14.06.2027', 14),
        (2, 3, N'05.07.2027', N'18.07.2027', 14),
        (3, 4, N'01.08.2027', N'10.08.2027', 10);

    PRINT N'Учебные данные добавлены.';
END
ELSE
    PRINT N'В базе уже есть данные, ничего не добавлено.';
GO

SELECT COUNT(*) AS SkolkoSotrudnikov FROM Sotrudniki;
GO
