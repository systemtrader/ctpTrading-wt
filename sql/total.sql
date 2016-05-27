SELECT
    sum(p)
FROM
    (
        SELECT
            CASE
        WHEN is_buy = 1 THEN
            (0 - real_price)
        WHEN is_buy = 0 THEN
            real_price
        END AS p
        FROM
            `order`
        WHERE
            id > 3324
        AND STATUS = 1
    ) AS tmp;




SELECT
    CASE
WHEN is_open = 0 THEN
    @l
WHEN is_open = 1 THEN
    @l :=@l + 1
END AS l,
 CASE
WHEN is_buy = 1 THEN
    (0 - real_price)
WHEN is_buy = 0 THEN
    real_price
END AS p
FROM
    `order` o,
    (SELECT @l := 0) tmp
WHERE
    id > 3324
AND STATUS = 1;





SELECT
    SUM(p)
FROM
    (
        SELECT
            CASE
        WHEN is_open = 0 THEN
            @l
        WHEN is_open = 1 THEN
            @l :=@l + 1
        END AS l,
        CASE
    WHEN is_buy = 1 THEN
        (0 - real_price)
    WHEN is_buy = 0 THEN
        real_price
    END AS p
    FROM
        `order` o,
        (SELECT @l := 0) tmp
    WHERE
        id > 3324
    AND STATUS = 1
    ) tmp2
GROUP BY
    l;









SELECT
    sum(g)
FROM
    (
        SELECT
            SUM(p) AS g
        FROM
            (
                SELECT
                    CASE
                WHEN is_open = 0 THEN
                    @l
                WHEN is_open = 1 THEN
                    @l :=@l + 1
                END AS l,
                CASE
            WHEN is_buy = 1 THEN
                (0 - real_price)
            WHEN is_buy = 0 THEN
                real_price
            END AS p
            FROM
                `order` o,
                (SELECT @l := 0) tmp
            WHERE
                id > 3324
            AND STATUS = 1
            ) tmp2
        GROUP BY
            l
    ) tmp3;
