.headers on
.mode column
SELECT a.Path,
       a.ModuleLabel,
       a.ModuleType,
       a.Vsize AS "Vsize (MiB)",
       a.DeltaVsize AS "DeltaVsize (MiB)",
       a.RSS AS "RSS (MiB)",
       a.DeltaRSS AS "DeltaRSS (MiB)",
       Run,
       SubRun,
       Event
FROM ModuleInfo AS a
WHERE (SELECT COUNT(*)+1
       FROM ModuleInfo AS b
       WHERE b.Path = a.Path
       AND b.ModuleLabel = a.ModuleLabel
       AND b.RSS > a.RSS) < 4
AND a.RSS > 0
ORDER BY a.Path, a.ModuleLabel ASC, a.RSS DESC;