DROP FUNCTION IF EXISTS `shell_escape`;
CREATE FUNCTION `shell_escape` ( command VARCHAR(255) )
	RETURNS VARCHAR(255)
		DETERMINISTIC
			RETURN
				REPLACE(
					REPLACE(
						REPLACE(
							REPLACE(
								REPLACE(
									REPLACE(command, '\\', '\\\\')
								, '\'', '\\\'')
							, '\`', '\\\`')
						, '$', '\\$')
					, '|', '\\|')
				, ' ', '\\ ');

DROP TRIGGER IF EXISTS `tweeter`;
CREATE TRIGGER `tweeter` AFTER INSERT ON `playlog`
	FOR EACH ROW
		SET @res = sys_exec(
			CONCAT_WS(' ', '/usr/local/bin/bingehacktweeter.rb',
				shell_escape(NEW.name),
				shell_escape(NEW.role),
				shell_escape(NEW.race),
				shell_escape(NEW.gender),
				shell_escape(NEW.align),
				shell_escape(NEW.deathlev),
				shell_escape(NEW.hp),
				shell_escape(NEW.maxhp),
				shell_escape(NEW.death))
		);
