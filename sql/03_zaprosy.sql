-- Запросы для проверки
USE OtpuskKompanii;
GO

-- сотрудники и их отделы
SELECT s.Tabel, s.Fio, o.Nazvanie AS Otdel, s.Norma
FROM Sotrudniki s
JOIN Otdely o ON o.Id = s.OtdelId
ORDER BY o.Nazvanie, s.Fio;
GO

-- все отпуска
SELECT o.Id, s.Fio, o.DataNachala, o.DataKonca, o.Dney
FROM Otpuska o
JOIN Sotrudniki s ON s.Id = o.SotrId
ORDER BY o.DataNachala;
GO

-- сколько дней уже занято и сколько осталось
SELECT s.Fio,
       s.Norma,
       ISNULL(SUM(o.Dney), 0) AS Zanyato,
       s.Norma - ISNULL(SUM(o.Dney), 0) AS Ostatok
FROM Sotrudniki s
LEFT JOIN Otpuska o ON o.SotrId = s.Id
GROUP BY s.Fio, s.Norma
ORDER BY s.Fio;
GO

-- сколько человек в каждом отделе
SELECT o.Nazvanie, COUNT(s.Id) AS Chel
FROM Otdely o
LEFT JOIN Sotrudniki s ON s.OtdelId = o.Id
GROUP BY o.Nazvanie;
GO
