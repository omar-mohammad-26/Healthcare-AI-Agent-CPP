export type AgentStep = {
    number: number
    name: string
    arguments?: unknown
    result?: unknown
    type?: string
}

export type AgentResponse = {
    status: string
    provider: string
    model: string
    answer: string
    step_count: number
    steps: AgentStep[]
}

type ApiErrorResponse = {
    message?: string
    code?: string
}

const apiBaseUrl =
    import.meta.env.VITE_API_BASE_URL?.trim() ||
    'http://localhost:8000'

const readJsonResponse = async (
    response: Response,
): Promise<unknown> => {
    const responseText = await response.text()

    if (!responseText) {
        return {}
    }

    try {
        return JSON.parse(responseText)
    } catch {
        throw new Error(
            'The Healthcare AI Agent returned an invalid JSON response.',
        )
    }
}

export const runHealthcareAgent = async (
    prompt: string,
): Promise<AgentResponse> => {
    const trimmedPrompt = prompt.trim()

    if (!trimmedPrompt) {
        throw new Error('The Agent prompt must not be empty.')
    }

    const response = await fetch(`${apiBaseUrl}/agent/run`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            prompt: trimmedPrompt,
        }),
    })

    const responseBody = await readJsonResponse(response)

    if (!response.ok) {
        const apiError = responseBody as ApiErrorResponse

        throw new Error(
            apiError.message ||
            `The Healthcare AI Agent request failed with status ${response.status}.`,
        )
    }

    const agentResponse = responseBody as Partial<AgentResponse>

    if (
        typeof agentResponse.answer !== 'string' ||
        !Array.isArray(agentResponse.steps)
    ) {
        throw new Error(
            'The Healthcare AI Agent returned an incomplete response.',
        )
    }

    return {
        status: agentResponse.status ?? 'success',
        provider: agentResponse.provider ?? 'unknown',
        model: agentResponse.model ?? 'unknown',
        answer: agentResponse.answer,
        step_count:
            typeof agentResponse.step_count === 'number'
                ? agentResponse.step_count
                : agentResponse.steps.length,
        steps: agentResponse.steps,
    }
}