-- Та же база для SQLite, если SQL Server нет
-- Можно открыть в DB Browser for SQLite

DROP TABLE IF EXISTS Otpuska;
DROP TABLE IF EXISTS Sotrudniki;
DROP TABLE IF EXISTS Otdely;

CREATE TABLE Otdely (
    Id INTEGER PRIMARY KEY,
    Nazvanie TEXT NOT NULL
);

CREATE TABLE Sotrudniki (
    Id INTEGER PRIMARY KEY,
    Fio TEXT NOT NULL,
    Tabel TEXT NOT NULL,
    OtdelId INTEGER NOT NULL,
    Norma INTEGER NOT NULL,
    FOREIGN KEY (OtdelId) REFERENCES Otdely(Id)
);

CREATE TABLE Otpuska (
    Id INTEGER PRIMARY KEY,
    SotrId INTEGER NOT NULL,
    DataNachala TEXT NOT NULL,
    DataKonca TEXT NOT NULL,
    Dney INTEGER NOT NULL,
    FOREIGN KEY (SotrId) REFERENCES Sotrudniki(Id)
);

INSERT INTO Otdely VALUES
(1, 'Бухгалтерия'),
(2, 'Отдел продаж'),
(3, 'Склад');

INSERT INTO Sotrudniki VALUES
(1, 'Иванов Иван Иванович', '1001', 1, 28),
(2, 'Петрова Анна Сергеевна', '1002', 1, 28),
(3, 'Сидоров Петр Алексеевич', '1003', 2, 28),
(4, 'Кузнецова Мария Олеговна', '1004', 2, 28),
(5, 'Волкова Елена Игоревна', '1005', 3, 28);

INSERT INTO Otpuska VALUES
(1, 2, '01.06.2027', '14.06.2027', 14),
(2, 3, '05.07.2027', '18.07.2027', 14),
(3, 4, '01.08.2027', '10.08.2027', 10);

SELECT s.Fio,
       s.Norma,
       s.Norma - IFNULL(SUM(o.Dney), 0) AS Ostatok
FROM Sotrudniki s
LEFT JOIN Otpuska o ON o.SotrId = s.Id
GROUP BY s.Id, s.Fio, s.Norma
ORDER BY s.Fio;
