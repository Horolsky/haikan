----------------------
-- unit testing module
----------------------

TestSuite = {
    name = "",
    _tests = {},
    _reports = {},
    _passed = 0,
    _failed = 0
}


TestSuite.__index = TestSuite


function TestSuite:__close()
    print(self:report())
end


function TestSuite:_append_report(report)
    local N = #self._reports
    self._reports[N+1] = report
    if report.passed then
        self._passed = self._passed + 1
    else
        self._failed = self._failed + 1
    end
end


function TestSuite:report()
    local template = string.rep ("\t%s", #self._reports, ",\n")
    local reports = string.format(template, table.unpack(self._reports))
    local msg = string.format("\tname = \"%s\",\n\treports = {\n%s\n\t}", self.name, reports)

    msg = msg .. ",\n\tpassed = " .. tostring(self._passed)
    msg = msg .. ",\n\tfailed = " .. tostring(self._failed)
    return "{\n" .. msg .. "\n}"
end

function TestSuite:test(name, runner)
    assert(type(name) == "string")
    assert(type(runner) == "function")

    table.insert(self._tests, {name=name, runner=runner})
end


function TestSuite:new(name)
    assert(type(name) == "string")
    local ts =  setmetatable({}, self)
    ts.name = name
    return ts
end

function TestSuite:run()
    local runned_tests = 0
    local failed_tests = 0

    local color = {
        reset ="\27[0m" ,
        red   ="\27[31m",
        green ="\27[32m",
    }

    for _, test in ipairs(self._tests) do
        local testname, runner = test.name, test.runner
        print(color.reset..'RUNNIG '..self.name.."."..testname.."...")
        local passed, message = pcall(runner)
        runned_tests = runned_tests + 1
        if not passed then
            failed_tests = failed_tests + 1
            print(color.red.."FAILED: "..message.."\n")
        else
            print(color.green.."OK!\n")
        end
    end
    print("\27[0m" .. self.name .. " complete")
    print("tests runned:\t" .. runned_tests)
    if (failed_tests > 0) then
        print(color.red.."tests failed:\t"..failed_tests)
    else
        print(color.green.."all tests passed!")
    end
    print(color.reset .. "Version: " .. _VERSION)
    return failed_tests
end

